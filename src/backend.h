//
// Created by kyyou on 2026/2/17.
//

#ifndef LEARNING_CPP_BACKEND_H
#define LEARNING_CPP_BACKEND_H

#include <QByteArray>
#include <QImage>
#include <QColor>
#include <QList>


// 取色器
namespace color_picker {
    QColor pick_color(int x, int y);
    QString color_to_hex(const QColor &color);
    QString color_to_rgb(const QColor &color);
    QString color_to_hsl(const QColor &color);
    QString color_to_hsv(const QColor &color);
    QString color_to_hwb(const QColor &color);
    QString color_to_ncol(const QColor &color);
    QString color_to_ciexyz(const QColor &color);
    QString color_to_cielab(const QColor &color);
    QString color_to_oklab(const QColor &color);
    QString color_to_oklch(const QColor &color);
    QString color_to_vec4(const QColor &color);
    QString color_to_dec(const QColor &color);
    QString color_to_hex_int(const QColor &color);
}

// Image Resizer
namespace image_resizer {
    enum ResizeMode { Fill, Fit, Stretch };
    enum SizeUnit { CM, Inch, Percent, Pixel };
    
    struct ResizeConfig {
        int width;
        int height;
        ResizeMode mode;
        SizeUnit unit;
        bool keep_aspect;
    };
    
    bool resize_image(const QString &source_path, const QString &output_path, const ResizeConfig &config);
    bool batch_resize(const QStringList &source_paths, const QString &output_dir, const ResizeConfig &config);
    bool resize_with_fallback(const QString &source_path, const QString &output_path, 
                             const ResizeConfig &config, const QString &fallback_format);
}

// Rename工具
namespace batch_rename {
    struct RenameResult {
        QString original;
        QString new_name;
        bool success;
        QString error;
    };
    
    QList<RenameResult> preview_rename(const QStringList &files, const QString &pattern);
    QList<RenameResult> preview_replace(const QStringList &files, const QString &search, const QString &replace);
    QList<RenameResult> preview_regex_rename(const QStringList &files, const QString &pattern, const QString &replacement);
    bool apply_rename(const QList<RenameResult> &results);
    bool undo_rename();
}

// 进制转换器
namespace base_converter {
    QString convert_base(const QString &number, int from_base, int to_base);
    bool is_valid_number(const QString &number, int base);
    int get_max_base();
}

// 时间戳转换
namespace timestamp_converter {
    QString timestamp_to_datetime(qint64 timestamp, const QString &timezone);
    qint64 datetime_to_timestamp(const QString &datetime, const QString &timezone);
    QStringList get_available_timezones();
    qint64 get_current_timestamp();
    qint64 components_to_timestamp(int year, int month, int day, int hour, int minute, int second, const QString &timezone);
}

// Base64
namespace base64_tool {
    QString encode(const QString &text);
    QString decode(const QString &base64_text);
    QString encode_bytes(const QByteArray &data);
    QByteArray decode_to_bytes(const QString &base64_text);
    bool is_valid_base64(const QString &text);
}

// 文本比对
namespace text_diff {
    struct DiffResult {
        QString original;
        QString modified;
        QList<QPair<int, int>> diff_ranges;
    };
    
    DiffResult compare(const QString &text1, const QString &text2);
    QString generate_diff_html(const DiffResult &result);
}

// GZip
namespace gzip_tool {
    QByteArray compress(const QByteArray &data);
    QByteArray decompress(const QByteArray &data);
    bool compress_file(const QString &source_path, const QString &output_path);
    bool decompress_file(const QString &source_path, const QString &output_path);
}

// URL编解码
namespace url_tool {
    QString encode(const QString &text);
    QString decode(const QString &encoded_text);
    QString encode_component(const QString &text);
    QString decode_component(const QString &encoded_text);
}

// 哈希/校验和生成器
namespace hash_tool {
    QString md5(const QString &text);
    QString sha1(const QString &text);
    QString sha256(const QString &text);
    QString sha512(const QString &text);
    QString crc32(const QString &text);
    QString file_md5(const QString &file_path);
    QString file_sha256(const QString &file_path);
}

// 密码生成器
namespace password_gen
{
    struct PasswordConfig
    {
        int length;
        bool use_uppercase,
             use_lowercase,
             use_numbers,
             use_symbols,
             exclude_ambiguous;
        QString custom_symbols;
    };
    
    QString generate_password(const PasswordConfig &config);
    double calculate_entropy(const QString &password);
    QString evaluate_strength(double entropy);
}

// 设置管理
namespace settings
{
    bool load_settings();
    void cleanup_settings();
    bool save_settings();
    bool get_shortcut_enabled();
    void set_shortcut_enabled(bool enabled);
    QString get_tool_config(const QString &tool_name);
    void set_tool_config(const QString &tool_name, const QString &config);
}

namespace common_backend {
    void save_config();
}

#endif //LEARNING_CPP_BACKEND_H
