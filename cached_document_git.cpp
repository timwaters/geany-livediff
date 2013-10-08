#include <boost/filesystem.hpp>
#include <geanyplugin.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <libgen.h>
#include <stdexcept>
#include "cached_document_git.h"
#include "cache_util.h"
using namespace std;

gld::CachedDocumentGit::CachedDocumentGit(GeanyDocument* doc, boost::shared_ptr<GitRepo> _repo) :
  CachedDocument(doc), repo(_repo)
{
}

gld::CachedDocumentGit::~CachedDocumentGit(void)
{
}

void gld::CachedDocumentGit::cache(void)
{
  if (text_lines.empty() || !repo.get()->check_head()) {
    cout << "caching " << geany_document->real_path << endl;
    std::string str;
    repo.get()->get_file(geany_document->real_path, str);
  
    text_lines.clear();    
    std::stringstream ss(str);
    std::string line;
    while (std::getline(ss, line, '\n'))
      text_lines.push_back(line);
  
    update_markers();
  }
}

gld::CachedDocumentGit* gld::CachedDocumentGit::attempt_create(GeanyDocument* doc)
{
  char path[PATH_MAX];
  string real_path = doc->real_path;
  string base_path = dirname(const_cast<char*>(real_path.c_str()));

  boost::shared_ptr<GitRepo> repo_ptr;
  if (git_repository_discover(path, PATH_MAX, base_path.c_str(), 0, NULL) == 0) {
    // if this belongs to a git repo, check if other open document is of the same repo and reuse it
    for (CachedDocuments::iterator it = cached_documents.begin(); it != cached_documents.end(); it++) {
      CachedDocumentGit* other_doc_ptr = dynamic_cast<CachedDocumentGit*>(it->second.get());
      
      if (other_doc_ptr && other_doc_ptr->repo->path() == path) {
        cout << "reusing repo ptr at " << other_doc_ptr->repo->path() << endl;
        repo_ptr = other_doc_ptr->repo;
        break;
      }
    }
    if (!repo_ptr) {
      cout << "creating new repo for " << base_path << endl;
      repo_ptr = boost::shared_ptr<GitRepo>(new GitRepo(base_path));
    }

    if (repo_ptr.get()->find_file(doc->real_path)) 
      return new CachedDocumentGit(doc, repo_ptr);
    else {
      cout << "this file does not belong to repo" << endl;
      return NULL;
    }
  }
  else return NULL;
}

gld::GitRepo::GitRepo(const std::string& path)
{
  cout << "repo path: " << path.c_str() << endl;
  git_repository_open(&repo, path.c_str());
  git_reference_name_to_id(&head_oid, repo, "HEAD");
}

gld::GitRepo::~GitRepo(void)
{
  cout << "closed repository" << path().c_str() << endl;
  git_repository_free(repo);
}

std::string gld::GitRepo::path(void)
{
  return git_repository_path(repo);
}

bool gld::GitRepo::check_head(void)
{
  git_oid oid;
  git_reference_name_to_id(&oid, repo, "HEAD");
  
  char oid_name[GIT_OID_HEXSZ+1];
  git_oid_tostr(oid_name, GIT_OID_HEXSZ+1, &head_oid);
  cout << "old head: " << oid_name << endl;
  git_oid_tostr(oid_name, GIT_OID_HEXSZ+1, &oid);
  cout << "cur head: " << oid_name << endl;
  
  if (git_oid_cmp(&oid, &head_oid) == 0) return true;
  else {
    git_oid_cpy(&head_oid, &oid);
    return false;
  }
}

git_object* gld::GitRepo::find_file(const string& document_path_str)
{
  boost::filesystem::path document_path = boost::filesystem::canonical(document_path_str);
  boost::filesystem::path repo_path = boost::filesystem::canonical(path());
  
  boost::filesystem::path::iterator it1, it2;
  for(it1 = document_path.begin(), it2 = repo_path.begin(); it1 != document_path.end() && it2 != repo_path.end(); ++it1, ++it2)
  {
    if (*it1 != *it2) break;
  }
  boost::filesystem::path final_path;
  while(it1 != document_path.end()) {
    final_path /= *it1;
    ++it1;
  }
  
  cout << "document_path: " << document_path << endl;
  cout << "repo_path: " << repo_path << endl;
  cout << "final_path: " << final_path << endl;

  string spec = string("HEAD:") + final_path.c_str();
  git_object* obj;
  git_revparse_single(&obj, repo, spec.c_str());
  if (!obj || git_object_type(obj) != GIT_OBJ_BLOB) return NULL;
  else return obj;
}

void gld::GitRepo::get_file(const string& document_path_str, string& str)
{
  git_object* obj = find_file(document_path_str);
  if (!obj) throw std::runtime_error(string("document ") + document_path_str + " is not on HEAD");
  str.assign((const char*)git_blob_rawcontent((git_blob*)obj), (size_t)git_blob_rawsize((git_blob*)obj));
}


