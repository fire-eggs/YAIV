//
// Created by kevin on 7/12/21.
//
// A vertical toolgroup
//
#ifndef YAIV_VTOOLGRP_H
#define YAIV_VTOOLGRP_H

#include "toolgrp.h"

class vtoolgrp : public toolgrp
{
public:
    vtoolgrp(dockgroup *d, bool floating, bool draggable, int w, int h, const char *l = nullptr);

private:
    void create_dockable_group() override;
    void create_fixed_group() override;
};

#endif //YAIV_VTOOLGRP_H
