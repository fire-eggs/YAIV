//
// Created by kevin on 5/23/21.
//

#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/filename.H>
#include <string>
#include <stdexcept>
#include <vector>

#include "danbooru.h"
#include "sqlite-amalgamation-3350500/sqlite3.h"

sqlite3 *db = nullptr;
Fl_Window *dbwin = nullptr;
Fl_Multiline_Output *txtout;

// connect to db; open window to view tags
void view_danbooru()
{
    // TODO browse / env variable
    int rc = sqlite3_open("/home/kevin/db2020/db2020.db", &db);
    if (rc)
    {
        fl_alert("Could not open danbooru database:\n%s", sqlite3_errmsg(db));
        return;
    }

    dbwin = new Fl_Window(200,500,"Danbooru Data");
    txtout = new Fl_Multiline_Output(5,5,190,490);
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

static int id_callback(void *data, int argc, char **argv, char **azColName)
{
    tags->push_back(std::string(*argv));
    return 0;
}

void update_danbooru(char *filename)
{
    if (!db) return; // db not open, nothing to do

    char name[500];
    char *zErrMsg = nullptr;

    // get image_id [name] from filename, w/o extension
    const char * nm = fl_filename_name(filename);
    const char *ext = strrchr(nm, '.');
    if (ext == nullptr)
        strcpy(name, nm);
    else
        strncpy(name,nm,ext-nm);

    unsigned long long image_id;
    try {
        image_id = std::stoll(nm);
    } catch (std::invalid_argument) {
        // TODO msg to outtxt
        txtout->value("Not a danbooru file!");
        return;
    }

    // get all tags from database as a vector of strings
    tags = new std::vector<std::string>();
    char query[500];
    sprintf(query, "select name from tags where tag_id in (select tag_id from imageTags where image_id = '%lld') order by name", image_id);
    auto rc = sqlite3_exec(db, query, id_callback, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        fl_alert("ID fetch for %s : %s", nm, zErrMsg);
        sqlite3_free(zErrMsg);
    }

    // output tags to txtout
    int tot_count=0;
    for (int i=0; i < tags->size(); i++)
    {
        std::string v = tags->at(i);
        tot_count += v.size() + 1;
    }

    char *taglist = (char *)malloc(tot_count + 2);
    taglist[0] = '\0';
    for (int i=0; i < tags->size(); i++)
    {
        std::string v = tags->at(i);
        strcat(taglist, v.c_str());
        strcat(taglist, "\n");
    }
    delete tags;
    txtout->value(taglist);
    free(taglist);
}
