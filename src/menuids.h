//
// Created by kevin on 7/30/21.
//

#ifndef YAIV_MENUIDS_H
#define YAIV_MENUIDS_H

enum
{
    MI_LOAD = 0,
    MI_COPYPATH,
    MI_GOTO,
#ifdef DANBOORU
    MI_DANBOORU,
#endif
#ifdef METADATA
    MI_METADATA,
#endif    
    MI_OPTIONS,

    MI_THEME_BLUE,
    MI_THEME_CLASSIC,
    MI_THEME_DARK,
    MI_THEME_GREYBIRD,
    MI_THEME_HIGHCONTRAST,
    MI_THEME_NATIVE,
    MI_THEME_OCEAN,
    MI_THEME_OLIVE,
    MI_THEME_ROSEGOLD,
    MI_THEME_TAN,

    MI_FAVS, // Must be last before MI_FAVx
    MI_FAV0,
    MI_FAV1,
    MI_FAV2,
    MI_FAV3,
    MI_FAV4,
    MI_FAV5,
    MI_FAV6,
    MI_FAV7,
    MI_FAV8,
    MI_FAV9,
    };

#endif //YAIV_MENUIDS_H
