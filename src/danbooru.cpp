//
// Created by kevin on 5/23/21.
//

#ifdef DANBOORU
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/filename.H>
#include <string>
#include <stdexcept>
#include <vector>

#include "danbooru.h"
#include "sqlite-amalgamation-3350500/sqlite3.h"

sqlite3 *db = nullptr;
Fl_Window *dbwin = nullptr;
Fl_Text_Editor *txtout;
Fl_Text_Buffer *textbuf;
Fl_Text_Buffer *stylebuf;

const char *labels = "Copyright:\nArtist:\nCharacters:\nMeta:\nTags:\n";

// TODO customization of text size
// TODO customization of text styles

#define TS 14 // default editor textsize

Fl_Text_Display::Style_Table_Entry  styletable[] = {	// Style table
        { FL_BLACK,      FL_TIMES,             TS }, // A - Plain
        { FL_RED,        FL_TIMES_BOLD,        TS }, // B - Line comments
        { FL_GREEN,      FL_HELVETICA_ITALIC,  TS }, // C - Block comments
        { FL_DARK_YELLOW,FL_TIMES,             TS }, // D - Strings
        { FL_DARK_RED,   FL_COURIER,           TS }, // E - Directives
        { FL_BLACK,      FL_TIMES_BOLD,   TS+1 }, // F - Types
        { FL_BLUE,       FL_COURIER_ITALIC,    TS }, // G - Keywords
};

void style_unfinished_cb(int, void*) {
}

// connect to db; open window to view tags
void view_danbooru(Prefs *prefs)
{
    // TODO browse mechanism
    char dbpath[1000];
    prefs->get("DanbooruDB", dbpath, "", 1000);

    int rc = sqlite3_open(dbpath, &db);
    if (rc)
    {
        fl_alert("Could not open danbooru database:\n%s", sqlite3_errmsg(db));
        return;
    }

    dbwin = new Fl_Window(200,500,"Danbooru Data");

    textbuf = new Fl_Text_Buffer;
    stylebuf = new Fl_Text_Buffer;

    txtout = new Fl_Text_Editor(5,5,190,490);
    txtout->textfont(FL_TIMES); // TODO options
    txtout->textsize(TS);       // TODO options
    txtout->buffer(textbuf);
    txtout->highlight_data(stylebuf, styletable,
                           sizeof(styletable) / sizeof(styletable[0]),
                           'A', style_unfinished_cb, 0);

    dbwin->resizable(txtout);
    dbwin->end();
    dbwin->show();
}

void shutdown_danbooru()
{
    sqlite3_close(db);
    if (dbwin)
        dbwin->hide();
}

std::vector<std::string> *tags;
std::vector<std::string> *tagcat;

void addTags(const char *category, const char *label, std::vector<std::string> *tags, std::vector<std::string> *tagcat,
             char *tmptxt, char *tmpsty);

static int id_callback(void *data, int argc, char **argv, char **azColName)
{
    tags->push_back(std::string(*argv));
    tagcat->push_back(std::string(*(argv+1)));
    return 0;
}

void addTags(const char *category, const char *label, std::vector<std::string> *tags,
             std::vector<std::string> *tagcat, char *tmptxt, char *tmpsty) {

    // Add the label, with style.
    strcat(tmptxt,label);
    for (int i=0; i < strlen(label); i++)
        strcat(tmpsty, "F");

    // default styled newline
    strcat(tmptxt, "\n");
    strcat(tmpsty, "A");

    for (int t=0; t < tagcat->size(); t++)
        // All tags of the desired category
        if (0 == strcmp(tagcat->at(t).c_str(), category)) {

            // add the tag
            strcat(tmptxt, tags->at(t).c_str());

            // style it
            for (int i=0; i< tags->at(t).size(); i++)
                strcat(tmpsty, "A");

            // default styled newline
            strcat(tmptxt, "\n");
            strcat(tmpsty, "A");
        }
}

void update_danbooru(char *filename)
{
    if (!db) return; // db not open, nothing to do

    char *zErrMsg = nullptr;

    // get image_id [name] from filename, w/o extension
    const char * nm = fl_filename_name(filename);
    const char *ext = strrchr(nm, '.');
    char name[500];
    memset(name, 0, 500);
    if (ext == nullptr)
        strcpy(name, nm);
    else
        strncpy(name,nm,ext-nm);

    unsigned long long image_id;
    size_t posEnd;
    try {
        image_id = std::stoll(name, &posEnd, 10);
    } catch (std::invalid_argument) {
        textbuf->text("Not a danbooru file!");
        stylebuf->text("BBBBBBBBBBBBBBBBBBBB");
        dbwin->redraw();
        return;
    }

    if (posEnd < strlen(name)) {
        textbuf->text("Not a danbooru file!");
        stylebuf->text("BBBBBBBBBBBBBBBBBBBB");
        dbwin->redraw();
        return;
    }

    // get all tags and categories from database as a vector of strings
    tags = new std::vector<std::string>();
    tagcat = new std::vector<std::string>();
    char query[500];
    sprintf(query, "select name,category from tags where tag_id in (select tag_id from imageTags where image_id = '%lld') order by category,name", image_id);
    auto rc = sqlite3_exec(db, query, id_callback, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        fl_alert("ID fetch for %s : %s", nm, zErrMsg);
        sqlite3_free(zErrMsg);
    }

    // 1. Determine total size of output text [tags, labels, newlines]
    int totsize = 0;
    for (int t=0; t<tags->size(); t++)
        totsize += tags->at(t).size() + 1;
    totsize += strlen(labels) + 1;

    // 2. allocate temp text/style bufs
    char *tmptxt = new char[totsize + 1];
    char *tmpsty = new char[totsize + 1];
    tmptxt[0] = '\0';
    tmpsty[0] = '\0';

    // 3. add tags of different category with appropriate (styled) label
    addTags("3","Copyright:",tags,tagcat,tmptxt,tmpsty);
    addTags("1","Artist:",tags,tagcat,tmptxt,tmpsty);
    addTags("4","Characters:",tags,tagcat,tmptxt,tmpsty);
    addTags("5","Meta:",tags,tagcat,tmptxt,tmpsty);
    addTags("0","Tags:",tags,tagcat,tmptxt,tmpsty);

    // 4. set output text/style
    textbuf->text(tmptxt);
    stylebuf->text(tmpsty);

    // 5. deallocate temp bufs
    delete[] tmptxt;
    delete[] tmpsty;

    dbwin->redraw();
}

#endif