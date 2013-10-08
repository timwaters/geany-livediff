#include <iostream>
#include <git2.h>
#include "cache_util.h"
#include "config.h"
using namespace std;
using namespace gld;

/* Geany Plugin definition */
PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("LiveDiff", "Displays added/deleted/modified line indicators in editor window, continously updated",
                "1.0", "v01d <phreakuencies@gmail.com>");

vector<GdkPixbuf*> gld::marker_icons;
GeanyFunctions* geany_functions;
GeanyData* geany_data;

/* Signal Handling */
static void on_document_open(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
  if (doc->real_path) {
    cout << "opened document " << doc->real_path << " " << doc << endl;
    cache_document(doc);
  }
}

static void on_document_close(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
  cout << "closed document: ";
  if (doc->real_path) {
    cout << doc->real_path << endl;
    uncache_document(doc);
  }
  else cout << "not real document" << endl;
}

static void on_document_reload(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
  cout << "realoaded document: ";
  if (doc->real_path) {
    cout << doc->real_path << endl;
    cached_documents[doc]->set_margins();
    cached_documents[doc]->cache();
  }
  else cout << "not real document" << endl;
}

static void on_document_save(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
  cout << "document saved: ";
  if (doc->real_path) {
    cout << doc->real_path << endl;
    cached_documents[doc]->set_margins();
  }
  else cout << "not real document" << endl;
}

static void on_document_activate(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
  cout << "activated document: ";
  if (doc->real_path) {
    cout << doc->real_path << endl;
    cached_documents[doc]->set_margins();
  }
  else cout << "not real document" << endl;
}

static gboolean editor_notify(GObject *obj, GeanyEditor *editor, SCNotification *nt, gpointer user_data)
{
  if (editor->document->real_path &&
      nt->nmhdr.code == SCN_MODIFIED &&
      (nt->modificationType & SC_MOD_INSERTTEXT || nt->modificationType & SC_MOD_DELETETEXT))
  {
    cout << "text changed" << endl;
    cached_documents[editor->document]->update_markers();
  }
  return FALSE;
}
    
extern "C" {
void plugin_init(GeanyData *data)
{
  git_threads_init();
  //GdkPixbuf *pix = gdk_pixbuf_new_from_file("abc.png", &error);
  marker_icons.resize(4);
  
  GError* load_error = NULL;
  marker_icons[ICON_ADDED] = gdk_pixbuf_new_from_file(FULL_ICON_PATH "/added.png", &load_error);
  marker_icons[ICON_MODIFIED] = gdk_pixbuf_new_from_file(FULL_ICON_PATH "/modified.png", &load_error);
  marker_icons[ICON_DELETED] = gdk_pixbuf_new_from_file(FULL_ICON_PATH "/deleted.png", &load_error);
  marker_icons[ICON_DELETED_MODIFIED] = gdk_pixbuf_new_from_file(FULL_ICON_PATH "/deleted_modified.png", &load_error);

  /* cache all opened documents */
  int i;
  foreach_document(i) { cache_document(documents[i]); }
}

void plugin_cleanup(void)
{
  clear_cache();
  for (int i = 0; i < marker_icons.size(); i++) g_object_unref(marker_icons[i]);
  git_threads_shutdown();
}

PluginCallback plugin_callbacks[] =
{
    { "document-open", (GCallback)&on_document_open, TRUE, NULL },
    { "document-close", (GCallback)&on_document_close, TRUE, NULL },
    { "document-reload", (GCallback)&on_document_reload, TRUE, NULL },
    { "document-save", (GCallback)&on_document_save, TRUE, NULL },
    { "document-activate", (GCallback)&on_document_activate, TRUE, NULL },
    { "editor-notify", (GCallback)&editor_notify, TRUE, NULL },
    { NULL, NULL, FALSE, NULL }
};
}


