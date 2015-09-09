#ifndef LIBREALSENSE_CPP_INCLUDE_GUARD
#define LIBREALSENSE_CPP_INCLUDE_GUARD

#include "rs.h"
#include "rsutil.h"

#include <cstdint>
#include <ostream>

namespace rs
{
    enum class stream : int32_t
    {
        depth                           = RS_STREAM_DEPTH,
        color                           = RS_STREAM_COLOR,
        infrared                        = RS_STREAM_INFRARED,
        infrared2                       = RS_STREAM_INFRARED2
    };

    enum class format : int32_t
    {
        any                             = RS_FORMAT_ANY,
        z16                             = RS_FORMAT_Z16,
        yuyv                            = RS_FORMAT_YUYV,
        rgb8                            = RS_FORMAT_RGB8,
        bgr8                            = RS_FORMAT_BGR8,
        rgba8                           = RS_FORMAT_RGBA8,
        bgra8                           = RS_FORMAT_BGRA8,
        y8                              = RS_FORMAT_Y8,
        y16                             = RS_FORMAT_Y16
    };

    enum class preset : int32_t
    {
        best_quality                    = RS_PRESET_BEST_QUALITY,
        largest_image                   = RS_PRESET_LARGEST_IMAGE,
        highest_framerate               = RS_PRESET_HIGHEST_FRAMERATE
    };

    enum class distortion : int32_t
    {
        none                            = RS_DISTORTION_NONE,
        modified_brown_conrady          = RS_DISTORTION_MODIFIED_BROWN_CONRADY,
        inverse_brown_conrady           = RS_DISTORTION_INVERSE_BROWN_CONRADY
    };

    enum class option : int32_t
    {
        f200_laser_power                = RS_OPTION_F200_LASER_POWER,
        f200_accuracy                   = RS_OPTION_F200_ACCURACY,
        f200_motion_range               = RS_OPTION_F200_MOTION_RANGE,
        f200_filter_option              = RS_OPTION_F200_FILTER_OPTION,
        f200_confidence_threshold       = RS_OPTION_F200_CONFIDENCE_THRESHOLD,
        f200_dynamic_fps                = RS_OPTION_F200_DYNAMIC_FPS,
        r200_lr_auto_exposure_enabled   = RS_OPTION_R200_LR_AUTO_EXPOSURE_ENABLED,
        r200_lr_gain                    = RS_OPTION_R200_LR_GAIN,
        r200_lr_exposure                = RS_OPTION_R200_LR_EXPOSURE,
        r200_emitter_enabled            = RS_OPTION_R200_EMITTER_ENABLED,
        r200_depth_control_preset       = RS_OPTION_R200_DEPTH_CONTROL_PRESET,
        r200_depth_units                = RS_OPTION_R200_DEPTH_UNITS,
        r200_depth_clamp_min            = RS_OPTION_R200_DEPTH_CLAMP_MIN,
        r200_depth_clamp_max            = RS_OPTION_R200_DEPTH_CLAMP_MAX,
        r200_disparity_mode_enabled     = RS_OPTION_R200_DISPARITY_MODE_ENABLED,
        r200_disparity_multiplier       = RS_OPTION_R200_DISPARITY_MULTIPLIER,
        r200_disparity_shift            = RS_OPTION_R200_DISPARITY_SHIFT         
    };

    class error : public std::runtime_error
    {
        std::string         function, args;
    public:
                            error(rs_error * err)                                                   : std::runtime_error(rs_get_error_message(err)), function(rs_get_failed_function(err)), args(rs_get_failed_args(err)) { rs_free_error(err); }

        const std::string & get_failed_function() const                                             { return function; }
        const std::string & get_failed_args() const                                                 { return args; }
    };

    class throw_on_error
    {
        rs_error *          err;
                            throw_on_error(const throw_on_error &)                                  = delete;
        throw_on_error &    operator = (const throw_on_error &)                                     = delete;
    public:
                            throw_on_error()                                                        : err() {}
                            #ifdef WIN32
                            ~throw_on_error()                                                       { if (err) throw error(err); }
                            #else
                            ~auto_error() noexcept(false)                                           { if (err) throw error(err); }
                            #endif
                            operator rs_error ** ()                                                 { return &err; }
    };

    struct int2
    { 
        int                 x,y;

        bool                operator == (const int2 & r) const                                      { return x==r.x && y==r.y; } 
    };

    struct float2
    {
        float               x,y;

        bool                operator == (const float2 & r) const                                    { return x==r.x && y==r.y; } 
    };

    struct float3
    { 
        float               x,y,z; 

        bool                operator == (const float3 & r) const                                    { return x==r.x && y==r.y && z==r.z; } 
    };

    struct float3x3
    { 
        float3              x,y,z;

        bool                operator == (const float3x3 & r) const                                  { return x==r.x && y==r.y && z==r.z; }
    };

    struct intrinsics
    {
        int2                image_size;
        float2              focal_length;
        float2              principal_point;
        float               distortion_coeff[5];
        distortion          distortion_model;

        float3              deproject(const float2 & pixel, float depth) const                      { float3 point; rs_deproject_pixel_to_point(&point.x, (const rs_intrinsics *)this, &pixel.x, depth); return point; }
        float2              project(const float3 & point) const                                     { float2 pixel; rs_project_point_to_pixel(&pixel.x, (const rs_intrinsics *)this, &point.x); return pixel; }
        bool                operator == (const intrinsics & r) const                                { return memcmp(this, &r, sizeof(intrinsics)) == 0; }
    };

    struct extrinsics
    {
        float3x3            rotation;
        float3              translation;

        bool                is_identity() const                                                     { return rotation == float3x3{{1,0,0},{0,1,0},{0,0,1}} && translation == float3{0,0,0}; }
        float3              transform(const float3 & point) const                                   { float3 p; rs_transform_point_to_point(&p.x, (const rs_extrinsics *)this, &point.x); return p; }
    };

    class device
    {
        rs_device *         dev;
    public:
                            device()                                                                : dev() {}
                            device(rs_device * dev)                                                 : dev(dev) {}
        explicit            operator bool() const                                                   { return !!dev; }
        rs_device *         get_handle() const                                                      { return dev; }

        const char *        get_name()                                                              { return rs_get_device_name(dev, throw_on_error()); }
        extrinsics          get_extrinsics(stream from, stream to)                                  { extrinsics extrin; rs_get_device_extrinsics(dev, (rs_stream)from, (rs_stream)to, (rs_extrinsics *)&extrin, throw_on_error()); return extrin; }
        float               get_depth_scale()                                                       { return rs_get_device_depth_scale(dev, throw_on_error()); }
        bool                supports_option(option o) const                                         { return !!rs_device_supports_option(dev, (rs_option)o, throw_on_error()); }

        void                enable_stream(stream s, int width, int height, format f, int fps)       { rs_enable_stream(dev, (rs_stream)s, width, height, (rs_format)f, fps, throw_on_error()); }
        void                enable_stream(stream s, preset preset)                                  { rs_enable_stream_preset(dev, (rs_stream)s, (rs_preset)preset, throw_on_error()); }
        bool                is_stream_enabled(stream s)                                             { return !!rs_stream_is_enabled(dev, (rs_stream)s, throw_on_error()); }
        intrinsics          get_stream_intrinsics(stream s)                                         { intrinsics intrin; rs_get_stream_intrinsics(dev, (rs_stream)s, (rs_intrinsics *)&intrin, throw_on_error()); return intrin; }
        format              get_stream_format(stream s)                                             { return (format)rs_get_stream_format(dev, (rs_stream)s, throw_on_error()); }
        int                 get_stream_framerate(stream s)                                          { return rs_get_stream_framerate(dev, (rs_stream)s, throw_on_error()); }

        void                start()                                                                 { rs_start_device(dev, throw_on_error()); }
        void                stop()                                                                  { rs_stop_device(dev, throw_on_error()); }
        bool                is_streaming()                                                          { return !!rs_device_is_streaming(dev, throw_on_error()); }

        void                set_option(option o, int value)                                         { rs_set_device_option(dev, (rs_option)o, value, throw_on_error()); }
        int                 get_option(option o)                                                    { return rs_get_device_option(dev, (rs_option)o, throw_on_error()); }

        void                wait_for_frames(int stream_bits)                                        { rs_wait_for_frames(dev, stream_bits, throw_on_error()); }
        int                 get_frame_number(stream s)                                              { return rs_get_frame_number(dev, (rs_stream)s, throw_on_error()); }
        const void *        get_frame_data(stream s)                                                { return rs_get_frame_data(dev, (rs_stream)s, throw_on_error()); }
    };

    class context
    {
        rs_context *        ctx;
                            context(const context &)                                                = delete;
        context &           operator = (const context &)                                            = delete;
    public:
                            context()                                                               : ctx(rs_create_context(RS_API_VERSION, throw_on_error())) {}

                            ~context()                                                              { rs_delete_context(ctx, nullptr); } // Deliberately ignore error on destruction
        rs_context *        get_handle() const                                                      { return ctx; }

        int                 get_device_count()                                                      { return rs_get_device_count(ctx, throw_on_error()); }
        device              get_device(int index)                                                   { return rs_get_device(ctx, index, throw_on_error()); }
    };

    inline std::ostream &   operator << (std::ostream & out, stream stream)                         { return out << rs_stream_to_string((rs_stream)stream); }
    inline std::ostream &   operator << (std::ostream & out, format format)                         { return out << rs_format_to_string((rs_format)format); }
    inline std::ostream &   operator << (std::ostream & out, preset preset)                         { return out << rs_preset_to_string((rs_preset)preset); }
    inline std::ostream &   operator << (std::ostream & out, distortion distortion)                 { return out << rs_distortion_to_string((rs_distortion)distortion); }
    inline std::ostream &   operator << (std::ostream & out, option option)                         { return out << rs_option_to_string((rs_option)option); }
}

static_assert(sizeof(rs::intrinsics) == sizeof(rs_intrinsics), "struct layout error");
static_assert(sizeof(rs::extrinsics) == sizeof(rs_extrinsics), "struct layout error");

#endif