// easing
#include <stdint.h>
#include <assert.h>
#include <math.h>

#define EASING_PI (3.14159265358979323846)
#define EASINGPI2 (EASING_PI / 2.0)

/// <summary>
/// Implementations the bounce ease out.
/// </summary>
/// <param name="p">The p.</param>
/// <returns></returns>
static double impl_bounce_ease_out(double p) {
    if (p < 4.0 / 11.0) {
        return (121.0 * p * p) / 16.0;
    }
    else if (p < 8.0 / 11.0) {
        return (363.0 / 40.0 * p * p) - (99.0 / 10.0 * p) + 17.0 / 5.0;
    }
    else if (p < 9.0 / 10.0) {
        return (4356.0 / 361.0 * p * p) - (35442.0 / 1805.0 * p) + 16061.0 / 1805.0;
    }
    else {
        return (54.0 / 5.0 * p * p) - (513.0 / 25.0 * p) + 268.0 / 25.0;
    }
}

// the type of aniamtion
enum ui_impl_animation_type {
    // #1
    ui_impl_linear = 0,
    // #2
    ui_impl_quad_ease_in,
    ui_impl_quad_ease_out,
    ui_impl_quad_ease_inout,
    // #3
    ui_impl_cubi_ease_in,
    ui_impl_cubi_ease_out,
    ui_impl_cubi_ease_inout,
    // #4
    ui_impl_quar_ease_in,
    ui_impl_quar_ease_out,
    ui_impl_quar_ease_inout,
    // #5
    ui_impl_quin_ease_in,
    ui_impl_quin_ease_out,
    ui_impl_quin_ease_inout,
    // #sin
    ui_impl_sine_ease_in,
    ui_impl_sine_ease_out,
    ui_impl_sine_ease_inout,
    // #circle
    ui_impl_circ_ease_in,
    ui_impl_circ_ease_out,
    ui_impl_circ_ease_inout,
    // #exp
    ui_impl_expo_ease_in,
    ui_impl_expo_ease_out,
    ui_impl_expo_ease_inout,
    // #elastic
    ui_impl_elas_ease_in,
    ui_impl_elas_ease_out,
    ui_impl_elas_ease_inout,
    // #back
    ui_impl_back_ease_in,
    ui_impl_back_ease_out,
    ui_impl_back_ease_inout,
    // #bounce
    ui_impl_boun_ease_in,
    ui_impl_boun_ease_out,
    ui_impl_boun_ease_inout,
};

/// <summary>
/// UIs the easing function.
/// </summary>
/// <param name="type">The type.</param>
/// <param name="p">The p.</param>
/// <returns></returns>
double ui_easing_function(uint32_t type, double p) {
    assert((p >= 0.0 && p <= 1.0) && "bad argument");
    switch (type)
    {
    default:
        assert(!"type unknown");
    case ui_impl_linear:
        // 线性插值     f(x) = x
        return p;
    case ui_impl_quad_ease_in:
        // 平次渐入     f(x) = x^2
        return p * p;
    case ui_impl_quad_ease_out:
        // 平次渐出     f(x) =  -x^2 + 2x
        return -(p * (p - 2.0));
    case ui_impl_quad_ease_inout:
        // 平次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^2)
        // [0.5, 1.0]   f(x) = -(1/2)((2x-1)*(2x-3)-1) ; 
        return p < 0.5 ? (p * p * 2.0) : ((-2.0 * p * p) + (4.0 * p) - 1.0);
    case ui_impl_cubi_ease_in:
        // 立次渐入     f(x) = x^3;
        return p * p * p;
    case ui_impl_cubi_ease_out:
        // 立次渐出     f(x) = (x - 1)^3 + 1
    {
        double f = p - 1.0;
        return f * f * f + 1.0;
    }
    case ui_impl_cubi_ease_inout:
        // 立次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^3) 
        // [0.5, 1.0]   f(x) = (1/2)((2x-2)^3 + 2) 
        if (p < 0.5) {
            return p * p * p * 2.0;
        }
        else {
            double f = (2.0 * p) - 2.0;
            return 0.5 * f * f * f + 1.0;
        }
    case ui_impl_quar_ease_in:
        // 四次渐入     f(x) = x^4
        return  p * p *  p * p;
    case ui_impl_quar_ease_out:
        // 四次渐出     f(x) = 1 - (x - 1)^4
    {
        double f = (p - 1.0); f *= f;
        return 1.0 - f * f;
    }
    case ui_impl_quar_ease_inout:
        // 四次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^4)
        // [0.5, 1.0]   f(x) = -(1/2)((2x-2)^4 - 2)
        if (p < 0.5) {
            double f = p * p;
            return 8.0 * f * f;
        }
        else {
            double f = (p - 1.0); f *= f;
            return 1.0 - 8.0 * f * f;
        }
    case ui_impl_quin_ease_in:
        // 五次渐入     f(x) = x^5
    {
        double f = p * p;
        return f * f * p;
    }
    case ui_impl_quin_ease_out:
        // 五次渐出     f(x) = (x - 1)^5 + 1
    {
        double f = (p - 1.0);
        return f * f * f * f * f + 1.0;
    }
    case ui_impl_quin_ease_inout:
        // 五次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^5) 
        // [0.5, 1.0]   f(x) = (1/2)((2x-2)^5 + 2)
        if (p < 0.5) {
            double f = p * p;
            return 16.0 * f * f * p;
        }
        else {
            double f = ((2.0 * p) - 2.0);
            return  f * f * f * f * f * 0.5 + 1.0;
        }
    case ui_impl_sine_ease_in:
        // 正弦渐入     
        return sin((p - 1.0) * EASINGPI2) + 1.0;
    case ui_impl_sine_ease_out:
        // 正弦渐出     
        return sin(p * EASINGPI2);
    case ui_impl_sine_ease_inout:
        // 正弦出入     
        return 0.5 * (1.0 - cos(p * EASING_PI));
    case ui_impl_circ_ease_in:
        // 四象圆弧
        return 1.0 - sqrt(1.0 - (p * p));
    case ui_impl_circ_ease_out:
        // 二象圆弧
        return sqrt((2.0 - p) * p);
    case ui_impl_circ_ease_inout:
        // 圆弧出入
        if (p < 0.5) {
            return 0.5 * (1.0 - sqrt(1.0 - 4.0 * (p * p)));
        }
        else {
            return 0.5 * (sqrt(-((2.0 * p) - 3.0) * ((2.0 * p) - 1.0)) + 1.0);
        }
    case ui_impl_expo_ease_in:
        // 指数渐入     f(x) = 2^(10(x - 1))
        return (p == 0.0) ? (p) : (pow(2.0, 10.0 * (p - 1.0)));
    case ui_impl_expo_ease_out:
        // 指数渐出     f(x) =  -2^(-10x) + 1
        return (p == 1.0) ? (p) : (1.0 - pow(2.0, -10.0 * p));
    case ui_impl_expo_ease_inout:
        // 指数出入
        // [0,0.5)      f(x) = (1/2)2^(10(2x - 1)) 
        // [0.5,1.0]    f(x) = -(1/2)*2^(-10(2x - 1))) + 1 
        if (p == 0.0 || p == 1.0) return p;
        if (p < 0.5) {
            return 0.5 * pow(2.0, (20.0 * p) - 10.0);
        }
        else {
            return -0.5 * pow(2.0, (-20.0 * p) + 1.0) + 1.0;
        }
    case ui_impl_elas_ease_in:
        // 弹性渐入
        return sin(13.0 * EASINGPI2 * p) * pow(2.0, 10.0 * (p - 1.0));
    case ui_impl_elas_ease_out:
        // 弹性渐出
        return sin(-13.0 * EASINGPI2 * (p + 1.0)) * pow(2.0, -10.0 * p) + 1.0;
    case ui_impl_elas_ease_inout:
        // 弹性出入
        if (p < 0.5) {
            return 0.5 * sin(13.0 * EASINGPI2 * (2.0 * p)) *pow(2.0, 10.0 * ((2.0 * p) - 1.0));
        }
        else {
            return 0.5 * (sin(-13.0 * EASINGPI2 * ((2.0 * p - 1.0) + 1.0)) * pow(2.0, -10.0 * (2.0 * p - 1.0)) + 2.0);
        }
    case ui_impl_back_ease_in:
        // 回退渐入
        return  p * p * p - p * sin(p * EASING_PI);
    case ui_impl_back_ease_out:
        // 回退渐出
    {
        double f = (1.0 - p);
        return 1.0 - (f * f * f - f * sin(f * EASING_PI));
    }
    case ui_impl_back_ease_inout:
        // 回退出入
        if (p < 0.5) {
            double f = 2.0 * p;
            return 0.5 * (f * f * f - f * sin(f * EASING_PI));
        }
        else {
            double f = (1.0 - (2 * p - 1.0));
            return 0.5 * (1.0 - (f * f * f - f * sin(f * EASING_PI))) + 0.5;
        }
    case ui_impl_boun_ease_in:
        // 反弹渐入
        return 1.0 - impl_bounce_ease_out(1.0 - p);
    case ui_impl_boun_ease_out:
        // 反弹渐出
        return impl_bounce_ease_out(p);
    case ui_impl_boun_ease_inout:
        // 反弹出入
        if (p < 0.5) {
            return 0.5 * (1.0 - impl_bounce_ease_out(1.0 - (p*2.0)));
        }
        else {
            return 0.5 * impl_bounce_ease_out(p * 2.0 - 1.0) + 0.5;
        }
    }
}