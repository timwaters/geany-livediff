#include <sstream>
#include <fstream>
#include "cache_util.h"
#include "cached_document_git.h"
using namespace std;

gld::CachedDocuments gld::cached_documents;

/* global functions */
bool gld::cache_document(GeanyDocument* doc, bool force)
{
  CachedDocuments::iterator it = cached_documents.find(doc);
  if (it == cached_documents.end()) {
    CachedDocumentGit* doc_git = CachedDocumentGit::attempt_create(doc);
    if (doc_git) cached_documents[doc] = boost::shared_ptr<CachedDocumentGit>(doc_git);
    else
      cached_documents[doc] = boost::shared_ptr<CachedDocument>(new CachedDocument(doc));
  }
    
  if (force || it == cached_documents.end())
  {
    cached_documents[doc]->cache();
    return true;
  }
  else return false;
}

void gld::uncache_document(GeanyDocument* doc) {
  CachedDocuments::iterator it = cached_documents.find(doc);
  if (it != cached_documents.end()) { cached_documents.erase(it); cout << "erased cached document" << endl; }
}

void gld::check_sources(void)
{
  for (CachedDocuments::iterator it = cached_documents.begin(); it != cached_documents.end(); ++it)
    (it->second)->check_source();
}

void gld::clear_cache(void) {
  cached_documents.clear();
}
