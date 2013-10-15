#ifndef __CACHED_DOCUMENT_H__
#define __CACHED_DOCUMENT_H__

#include <vector>
#include <string>

namespace gld {
  enum LineMarker { LINE_ADDED = 16, LINE_DELETED = 17, LINE_MODIFIED = 18, LINE_DELETED_MODIFIED = 19 };
  
  class CachedDocument {
    public:
      CachedDocument(GeanyDocument* geany_document);
      virtual ~CachedDocument(void);
      
      GeanyDocument* geany_document;
      //DocumentType type;
      std::vector<std::string> text_lines;

      virtual void cache(void);
      void update_markers(void);

      void set_margins(void);
      virtual void check_source(void);

      //static DocumentType get_type(GeanyDocument* doc);

    private:
      void set_marker(int line, LineMarker flag);
      void set_markers_so_far(int& current_line, int& added_lines, int& deleted_lines);
      void clear_markers(void);
      
      void unset_margins(void);

      CachedDocument(void);
  };
}

#endif
