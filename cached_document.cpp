#include <geanyplugin.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "cached_document.h"
#include "cache_util.h"
using namespace std;
using dtl::Diff;
using dtl::elemInfo;
using dtl::uniHunk;


extern GeanyFunctions* geany_functions;

gld::CachedDocument::CachedDocument(GeanyDocument* _geany_document) :
  geany_document(_geany_document)
{
  set_margins();
}

gld::CachedDocument::~CachedDocument(void)
{
  if (geany_document)
    unset_margins();
}

void gld::CachedDocument::set_margins(void)
{
  ScintillaObject* sci = geany_document->editor->sci;
  
  /* enable margin 3 for this plugin: set mask only for the symbols used */
  scintilla_send_message(sci, SCI_SETMARGINWIDTHN, 3, 16);
  scintilla_send_message(sci, SCI_SETMARGINTYPEN, 3, SC_MARGIN_SYMBOL);
  int mask = (1 << LINE_ADDED) | (1 << LINE_DELETED) | (1 << LINE_MODIFIED) | (1 << LINE_DELETED_MODIFIED);
  scintilla_send_message(sci, SCI_SETMARGINMASKN, 3, mask);

  /* restrict geany's mask to not include our markers */
  int old_mask = scintilla_send_message(sci, SCI_GETMARGINMASKN, 1, 0);
  old_mask &= ~mask;
  scintilla_send_message(sci, SCI_SETMARGINMASKN, 1, old_mask);

  /* setup RGBA marker sizes */
  int width = gdk_pixbuf_get_width(marker_icons[0]);
  int height = gdk_pixbuf_get_height(marker_icons[0]);
  scintilla_send_message(sci, SCI_RGBAIMAGESETWIDTH, width, 0);
  scintilla_send_message(sci, SCI_RGBAIMAGESETHEIGHT, height, 0);
  /*int text_height = scintilla_send_message(editor->sci, SCI_TEXTHEIGHT, 0, 0);
  int scale = (int)((height / (float)text_height) * 100);
  cout << "scale: " << scale << " text: " << text_height << " height: " << height << endl;
  scintilla_send_message(editor->sci, SCI_RGBAIMAGESETSCALE, scale, 0);*/

  /* define RGBA markers */
  //scintilla_send_message(editor->sci, SCI_MARKERDEFINERGBAIMAGE, LINE_ADDED, (long int)gdk_pixbuf_get_pixels(icons[ICON_ADDED]));
  /*scintilla_send_message(editor->sci, SCI_MARKERDEFINE, LINE_ADDED,  SC_MARK_UNDERLINE);
  scintilla_send_message(editor->sci, SCI_MARKERDEFINE, LINE_MODIFIED, SC_MARK_FULLRECT);*/
  /*scintilla_send_message(editor->sci, SCI_MARKERDEFINE, LINE_DELETED, SC_MARK_CHARACTER + 175);
  scintilla_send_message(editor->sci, SCI_MARKERDEFINE, LINE_DELETED_MODIFIED, SC_MARK_CHARACTER + 175);*/

  /*scintilla_send_message(editor->sci, SCI_MARKERSETBACK, LINE_ADDED, (0 << 16) | (255 << 8) | 0);
  scintilla_send_message(editor->sci, SCI_MARKERSETBACK, LINE_MODIFIED, (255 << 16) | (128 << 8) | 255);
  scintilla_send_message(editor->sci, SCI_MARKERSETBACK, LINE_DELETED_MODIFIED, (255 << 16) | (128 << 8) | 255);
  scintilla_send_message(editor->sci, SCI_MARKERSETFORE, LINE_DELETED, (255 << 16) | (0 << 8) | 0);
  scintilla_send_message(editor->sci, SCI_MARKERSETFORE, LINE_DELETED_MODIFIED, (255 << 16) | (0 << 8) | 0);*/

  scintilla_send_message(sci, SCI_MARKERDEFINERGBAIMAGE, LINE_ADDED, (long int)gdk_pixbuf_get_pixels(marker_icons[ICON_ADDED]));
  scintilla_send_message(sci, SCI_MARKERDEFINERGBAIMAGE, LINE_MODIFIED, (long int)gdk_pixbuf_get_pixels(marker_icons[ICON_MODIFIED]));
  scintilla_send_message(sci, SCI_MARKERDEFINERGBAIMAGE, LINE_DELETED, (long int)gdk_pixbuf_get_pixels(marker_icons[ICON_DELETED]));
  scintilla_send_message(sci, SCI_MARKERDEFINERGBAIMAGE, LINE_DELETED_MODIFIED, (long int)gdk_pixbuf_get_pixels(marker_icons[ICON_DELETED_MODIFIED]));
}

void gld::CachedDocument::unset_margins(void)
{
  clear_markers();
  scintilla_send_message(geany_document->editor->sci, SCI_SETMARGINWIDTHN, 3, 0);
}

void gld::CachedDocument::clear_markers(void)
{
  ScintillaObject* sci = geany_document->editor->sci;
  scintilla_send_message(sci, SCI_MARKERDELETEALL, LINE_ADDED, 0);
  scintilla_send_message(sci, SCI_MARKERDELETEALL, LINE_DELETED, 0);
  scintilla_send_message(sci, SCI_MARKERDELETEALL, LINE_DELETED_MODIFIED, 0);
  scintilla_send_message(sci, SCI_MARKERDELETEALL, LINE_MODIFIED, 0);
}

void gld::CachedDocument::check_source(void)
{
  // do nothing, geany auto-detects files modified on disk
}

void gld::CachedDocument::cache(void)
{
  cout << "caching " << geany_document->real_path << endl;
  text_lines.clear();
  std::ifstream f(geany_document->real_path);
  while(f) {
    text_lines.push_back(std::string());
    getline(f, text_lines.back());
  }
  update_markers();
}

void gld::CachedDocument::update_markers(void)
{
  ScintillaObject* sci = geany_document->editor->sci;

  /* clear previous markers */
  clear_markers();

  /* read current buffer as lines */
  const char* new_text = (gchar*)scintilla_send_message(sci, SCI_GETCHARACTERPOINTER, 0, 0);
  vector<string> new_lines;
  std::stringstream ss(new_text);
  std::string line;
  while (std::getline(ss, line, '\n')) {
    new_lines.push_back(line);
  }

  /* compute diff between cached version and current */
  Diff<string> diff(text_lines, new_lines);
  diff.onHuge();
  diff.compose();
  diff.composeUnifiedHunks();

  /* go over each hunk */
  const vector< uniHunk<sesElem> >& hunks = diff.getUniHunks();
  cout << "found " << hunks.size() << " hunks" << endl;
  for (uint i = 0; i < hunks.size(); i++) {
    cout << "hunk " << i << endl;
    int first_line = (hunks[i].c - 1) + hunks[i].common[0].size();
    cout << "a,b,c,d " << hunks[i].a << " " << hunks[i].b << " " << hunks[i].c << " " << hunks[i].d << endl;
    cout << "lines " << first_line << " to " << (first_line + hunks[i].change.size() - 1) << " modified" << endl;
    cout << "changes: " << endl;

    int deleted_lines = 0, added_lines = 0;
    int current_line = first_line;
    for (uint j = 0; j < hunks[i].change.size(); j++) {
      if (hunks[i].change[j].second.type == dtl::SES_DELETE) {
        cout << "deleted part" << endl;
        if (added_lines > 0) set_markers_so_far(current_line, added_lines, deleted_lines);
        deleted_lines++;
      }
      else if (hunks[i].change[j].second.type == dtl::SES_ADD) {
        cout << "added part" << endl;
        added_lines++;
      }
      else if (hunks[i].change[j].second.type == dtl::SES_COMMON) {
        cout << "common part" << endl;
        set_markers_so_far(current_line, added_lines, deleted_lines);
        current_line++;
      }
    }
    set_markers_so_far(current_line, added_lines, deleted_lines);
  }
}

void gld::CachedDocument::set_marker(int line, LineMarker flag)
{
  int marker_number = (int)flag;
  scintilla_send_message(geany_document->editor->sci, SCI_MARKERADD, line, marker_number);
}

void gld::CachedDocument::set_markers_so_far(int& current_line, int& added_lines, int& deleted_lines)
{
  cout << "printing status" << endl;
  if (deleted_lines == added_lines && deleted_lines != 0)
    for (int k = 0; k < deleted_lines; k++, current_line++) {
      cout << current_line << " c " << endl;
      set_marker(current_line, LINE_MODIFIED);
    }
  else if (deleted_lines > added_lines) {
    if (added_lines == 0) {
      cout << current_line << " d " << endl;
      set_marker(current_line, LINE_DELETED);
    }
      
    for (int k = 0; k < added_lines; k++, current_line++) {
      const char* flag = (k == 0 ? "DC" : "C");
      cout << current_line << " " << flag << endl;
      set_marker(current_line, (k == 0 ? LINE_DELETED_MODIFIED : LINE_MODIFIED));
    }
  }
  else {
    for (int k = 0; k < deleted_lines; k++, current_line++) {
      cout << current_line << " c " << endl;
      set_marker(current_line, LINE_MODIFIED);
    }
    for (int k = 0; k < added_lines - deleted_lines; k++, current_line++) {
      cout << current_line << " a " << endl;
      set_marker(current_line, LINE_ADDED);
    }
  }
  deleted_lines = added_lines = 0;
}
