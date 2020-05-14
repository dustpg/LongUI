#pragma once


// longui namespace
namespace LongUI {
    // control
    class UIControl;
    // Manager data for each control
    struct ManagerData {
        // next delete/init/update node
        UIControl*          next_delinitupd;
    };
}

