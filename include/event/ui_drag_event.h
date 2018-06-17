#pragma once

namespace LongUI {
    /// <summary>
    /// drag event
    /// </summary>
    enum DragEvent {
        // drag enter on this control
        Event_DragEnter,
        // drag over on this control
        Event_DragOver,
        // drag leave on this control
        Event_DragLeave,
        // drop data
        Event_Drop,
    };
}