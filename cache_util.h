#ifndef __CACHE_UTIL_H__
#define __CACHE_UTIL_H__

#include <geanyplugin.h>
#include <vector>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include "dtl/dtl.hpp"
#include "cached_document.h"

extern GeanyFunctions *geany_functions;

namespace gld {
  extern std::vector<GdkPixbuf*> marker_icons;
  typedef std::pair<std::string, dtl::elemInfo> sesElem;

  enum MarkIconID { ICON_ADDED, ICON_DELETED, ICON_MODIFIED, ICON_DELETED_MODIFIED };
  
  /* cached documents interface */
  bool cache_document(GeanyDocument* doc, bool force = false);
  void uncache_document(GeanyDocument* doc);
  void check_sources(void);
  void clear_cache(void);
  typedef boost::shared_ptr<CachedDocument> CachedDocumentPtr;
  typedef std::map<GeanyDocument*, CachedDocumentPtr> CachedDocuments;
  extern CachedDocuments cached_documents;
}

#endif
