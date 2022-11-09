#ifdef METADATA
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "metadata.h"

#include <exiv2/exiv2.hpp>

#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

#include "themes.h"

Fl_Window *mdwin = nullptr;
static Fl_Text_Display *txtout;
static Fl_Text_Buffer *textbuf;
static Fl_Text_Buffer *stylebuf;

#define TS 16 // default editor textsize

static Fl_Text_Display::Style_Table_Entry  styletable[] = {	// Style table
        { FL_BLACK,      FL_TIMES,             TS }, // A - Plain
        { FL_RED,        FL_TIMES_BOLD,        TS }, // B - Line comments
        { FL_GREEN,      FL_HELVETICA_ITALIC,  TS }, // C - Block comments
        { FL_DARK_YELLOW,FL_TIMES,             TS }, // D - Strings
        { FL_DARK_RED,   FL_COURIER,           TS }, // E - Directives
        { FL_BLACK,      FL_TIMES_BOLD,   TS+1 }, // F - Types
        { FL_BLUE,       FL_COURIER_ITALIC,    TS }, // G - Keywords
};


static bool first=true;
void metadata(const char *file)
{
    if (first)
    {
        Exiv2::XmpParser::initialize();
        ::atexit(Exiv2::XmpParser::terminate);
        first=false;
    }
    
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
    assert(image.get() != 0);
    image->readMetadata();
    
    Exiv2::ExifData &exifData = image->exifData();
    Exiv2::IptcData &iptcData = image->iptcData();
    
    printf("-----------------------------------\n");
/*    
    if (exifData.empty()) 
    {
        printf("No exif data\n");
    }
    if (iptcData.empty())
    {
        printf("No iptc data\n");
    }
    if (exifData.empty() && iptcData.empty())
        return;
*/    
    printf("Exif:\n");
    Exiv2::ExifData::const_iterator end = exifData.end();
    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) 
    {
        //const char* tn = i->typeName();
        //printf("typeName: %s\n", tn ? tn : "Unknown");
        printf("%s : %s\n", i->tagLabel().c_str(), i->toString().c_str());
    }
    
    printf("IPTC:\n");
    Exiv2::IptcData::const_iterator end2=iptcData.end();
    for (Exiv2::IptcData::const_iterator i = iptcData.begin(); i != end2; i++)
    {
        printf("%s : %s\n", i->tagLabel().c_str(), i->toString().c_str());
    }
    
    printf("XMP:\n");
    Exiv2::XmpData &xmpData = image->xmpData();
    for (Exiv2::XmpData::const_iterator md = xmpData.begin(); md != xmpData.end(); ++md) 
    {
        if (md->toString().length() != 0)
            printf("%s : %s\n", md->tagLabel().c_str(), md->toString().c_str());
    }    
}

void init_metadata()
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
}

void create_metadata(Prefs *prefs, Fl_Group *container) {

    textbuf = new Fl_Text_Buffer;
    stylebuf = new Fl_Text_Buffer;

    txtout = new Fl_Text_Display(container->x() + 1,container->y() + 1,
                                container->w()-1,container->h()-1);
    txtout->textfont(FL_TIMES); // TODO from prefs
    txtout->textsize(TS);       // TODO from prefs
    txtout->buffer(textbuf);
    txtout->hide_cursor();

    if (OS::is_dark_theme(OS::current_theme())) { // TODO use isDark
        styletable[0].color = FL_WHITE;
        styletable[5].color = FL_WHITE;
    }

    txtout->highlight_data(stylebuf, styletable,
                           sizeof(styletable) / sizeof(styletable[0]),
                           'A', nullptr, 0);

    container->add(txtout);
    //container->resizable(txtout);
    // HACK Prevents the dismiss/dragbar widgets from resizing
    container->parent()->resizable(txtout); // TODO shouldn't have to know about this?
}

void addTag(char *tmptxt, char *tmpsty, std::string label, std::string data)
{
    strcat(tmptxt, label.c_str());
    strcat(tmptxt, ": ");
    strcat(tmptxt, data.c_str());
    strcat(tmptxt, "\n");
    
    int len = label.length() + data.length() + 3;
    for (int i=0; i < len; i++)
        strcat(tmpsty, "A");
}

void update_metadata(const char *filepath) {
    
    static const char *labels = "Exif:\nIPTC:\nXMP:\n";
    
    try
    {
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filepath);
    image->readMetadata();
    
    Exiv2::ExifData &exifData = image->exifData();
    Exiv2::IptcData &iptcData = image->iptcData();
    Exiv2::XmpData &xmpData = image->xmpData();
    
    // Some images found to have improperly formatted XML in their XMP
    bool badXmp = !xmpData.count() && xmpData.xmpPacket().length();
    
    size_t totsize = 0;
    
    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != exifData.end(); ++i) 
        totsize += i->tagLabel().length() + i->toString().length() + 5;
    for (Exiv2::IptcData::const_iterator i = iptcData.begin(); i != iptcData.end(); ++i) 
        totsize += i->tagLabel().length() + i->toString().length() + 5;
    for (Exiv2::XmpData::const_iterator i = xmpData.begin(); i != xmpData.end(); ++i) 
        totsize += i->tagLabel().length() + i->toString().length() + 5;
    totsize += strlen(labels);
    if (badXmp)
        totsize += 15;
    
    char *tmptxt = new char[totsize + 1];
    char *tmpsty = new char[totsize + 1];
    memset(tmptxt, 0, totsize+1);
    memset(tmpsty, 0, totsize+1);

    strcat(tmptxt,"Exif:");
    for (int i=0; i < strlen("Exif:"); i++)
        strcat(tmpsty, "F");
    strcat(tmptxt, "\n");
    strcat(tmpsty, "A");

    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != exifData.end(); ++i) 
        addTag(tmptxt, tmpsty, i->tagLabel(), i->toString());

    strcat(tmptxt,"IPTC:");
    for (int i=0; i < strlen("IPTC:"); i++)
        strcat(tmpsty, "F");
    strcat(tmptxt, "\n");
    strcat(tmpsty, "A");
    
    for (Exiv2::IptcData::const_iterator i = iptcData.begin(); i != iptcData.end(); ++i) 
        addTag(tmptxt, tmpsty, i->tagLabel(), i->toString());

    strcat(tmptxt,"XMP:");
    for (int i=0; i < strlen("XMP:"); i++)
        strcat(tmpsty, "F");
    strcat(tmptxt, "\n");
    strcat(tmpsty, "A");

    if (badXmp)
    {
        strcat(tmptxt, "Error in XMP!\n");
        strcat(tmpsty, "BBBBBBBBBBBBBA");
    }
    else
    {
        for (Exiv2::XmpData::const_iterator i = xmpData.begin(); i != xmpData.end(); ++i) 
            addTag(tmptxt, tmpsty, i->tagLabel(), i->toString());
    }
    textbuf->text(tmptxt);
    stylebuf->text(tmpsty);

    delete[] tmptxt;
    delete[] tmpsty;
    }
    catch (Exiv2::Error& e) 
    {
    }
}

#endif // METADATA
