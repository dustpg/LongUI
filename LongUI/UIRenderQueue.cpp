#include "LongUI.h"


// 渲染队列 构造函数
LongUI::CUIRenderQueue::CUIRenderQueue(UIWindow* window) noexcept {
    m_unitLike.length = 0; m_unitLike.window = window;
    // get window
    // static_cast<UIWindow*>(m_unitLike.window)
}

// 渲染队列 析构函数
LongUI::CUIRenderQueue::~CUIRenderQueue() noexcept {

}

// 重置
void LongUI::CUIRenderQueue::Reset(uint32_t freq) noexcept {
    // 一样就不处理
    if (m_dwDisplayFrequency == freq) return;
    // 修改
    m_dwDisplayFrequency = freq;
    // 创建
    CUIRenderQueue::UNIT* data = nullptr;
    if (freq) {
        data = LongUI::CtrlAllocT(data, LongUIPlanRenderingTotalTime * freq);
        if (data) {
            for (auto i = 0u; i < LongUIPlanRenderingTotalTime * freq; ++i) {
                data[i].length = 0;
            }
        }
    }
    // TODO: 完成转化
    if (m_pUnitsDataBegin && data) {

    }
    // 释放
    if (m_pUnitsDataBegin) LongUI::CtrlFree(m_pUnitsDataBegin);
    // 转移
    if (data) {
        m_pUnitsDataBegin = data;
        m_pUnitsDataEnd = data + LongUIPlanRenderingTotalTime * freq;
        // XXX
        m_pCurrentUnit = data;
    }
    // 开始渲染
    m_dwStartTime = ::timeGetTime();
}

// ++ 操作符
void LongUI::CUIRenderQueue::operator++() noexcept {
    // 渲染队列模式
    if (m_pCurrentUnit) {
        ++m_pCurrentUnit;
        if (m_pCurrentUnit == m_pUnitsDataEnd) {
            m_pCurrentUnit = m_pUnitsDataBegin;
            // 检查误差
            register auto time = m_dwStartTime;
            m_dwStartTime = ::timeGetTime();
            time = m_dwStartTime - time;
            UIManager << DL_Hint << "Time Deviation: "
                << long(time) - long(LongUIPlanRenderingTotalTime * 1000)
                << " ms" << endl;
            // TODO: 时间校正
        }
    }
    // 简单模式
    else {
        assert(!"error");
    }
}

// 计划渲染
void LongUI::CUIRenderQueue::PlanToRender(float wait, float render, UIControl* ctrl) noexcept {
    // 保留刷新
    if (render != 0.0f) render += 0.1f;
    assert((wait + render) < float(LongUIPlanRenderingTotalTime) && "time overflow");
    // 当前窗口
    auto window = m_unitLike.window;
    // 设置单元
    auto set_unit = [window](UNIT* unit, UIControl* ctrl) noexcept {
        // 已经全渲染了就不干
        if (unit->length && unit->units[0] == window) {
            return;
        }
        // 单元满了就设置为全渲染
        if (unit->length == LongUIDirtyControlSize) {
            unit->length = 1;
            unit->units[0] = window;
            return;
        }
        // 获取真正窗口
        auto get_real_render_control = [window](UIControl* control) noexcept {
            // 获取真正
            while (control != window) {
                if (control->flags & Flag_RenderParent) control = control->parent;
                else break;
            }
            return control;
        };
        // 渲染窗口也设置为全渲染
        ctrl = get_real_render_control(ctrl);
        if (ctrl == window) {
            unit->length = 1;
            unit->units[0] = window;
            return;
        }
#if 0
        // 检查是否在单元里面
        register bool not_in = true;
        for (auto unit_ctrl = unit->units; unit_ctrl < unit->units + unit->length; ++unit_ctrl) {
            if (*unit_ctrl == ctrl) {
                not_in = false;
                break;
            }
        }
        // 不在单元里面就加入
        if (not_in) {
            unit->units[unit->length] = ctrl;
            ++unit->length;
        }
#else
        // 不在单元里面就加入
        if (std::none_of(unit->units, unit->units + unit->length, [ctrl](UIControl* unit) {
            return unit == ctrl;
        })) {
            unit->units[unit->length] = ctrl;
            ++unit->length;
        }
#endif
    };
    // 渲染队列模式
    if (m_pCurrentUnit) {
        // 时间片计算
        auto frame_offset = long(wait * float(m_dwDisplayFrequency));
        auto frame_count = long(render * float(m_dwDisplayFrequency)) + 1;
        auto start = m_pCurrentUnit + frame_offset;
        for (long i = 0; i < frame_count; ++i) {
            if (start >= m_pUnitsDataEnd) {
                start -= LongUIPlanRenderingTotalTime * m_dwDisplayFrequency;
            }
            set_unit(start, ctrl);
            ++start;
        }
    }
    // 简单模式
    else {
        assert(!"error");
    }
}