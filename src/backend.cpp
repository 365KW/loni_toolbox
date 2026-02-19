#include "backend.h"
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QSettings>
#include <QImage>
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <qfileinfo.h>
#include <QSaveFile>
#include <QTimeZone>
#include <QDateTime>
#include <random>
#include <cmath>
#include <QtZlib/zlib.h>
#include <QJsonObject>
#include <QDir>
#include <QFile>

#ifdef Q_OS_WIN
#include <windows.h>
#endif


// ==================== 取色器 ====================
namespace color_picker {
    QColor pick_color(const int x, const int y)
    {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (!screen) return {};
        
        const QPixmap pixmap = screen->grabWindow(0, x, y, 1, 1);
        const QImage image = pixmap.toImage();
        if (image.isNull()) return {};
        
        return {image.pixel(0, 0)};
    }
    
    QString color_to_hex(const QColor &color) {return color.name().toUpper();}
    
    QString color_to_rgb(const QColor &color)
    {
        return QString("rgb(%1, %2, %3)")
            .arg(color.red())
            .arg(color.green())
            .arg(color.blue());
    }
    
    QString color_to_hsl(const QColor &color)
    {
        int h, s, l;
        color.getHsl(&h, &s, &l);
        return QString("hsl(%1, %2%, %3%)").arg(h).arg(s / 2.55, 0, 'f', 1).arg(l / 2.55, 0, 'f', 1);
    }
    
    QString color_to_hsv(const QColor &color)
    {
        int h, s, v;
        color.getHsv(&h, &s, &v);
        return QString("hsv(%1, %2%, %3%)").arg(h).arg(s / 2.55, 0, 'f', 1).arg(v / 2.55, 0, 'f', 1);
    }
    
    QString color_to_hwb(const QColor &color)
    {
        int h, s, v;
        color.getHsv(&h, &s, &v);
        const double w = (100.0 - s) * v / 100.0,
                     b = 100.0 - v;
        return QString("hwb(%1, %2%, %3%)").arg(h).arg(w, 0, 'f', 1).arg(b, 0, 'f', 1);
    }
    
    QString color_to_ncol(const QColor &color)
    {
        int h, s, l;
        color.getHsl(&h, &s, &l);
        QString name;
        if (h < 30||h > 330) name = "R";
        else if (h < 90) name = "Y";
        else if (h < 150) name = "G";
        else if (h < 210) name = "C";
        else if (h < 270) name = "B";
        else if (h < 330) name = "M";
        return QString("ncol(%1, %2%, %3%)").arg(name).arg(s / 2.55, 0, 'f', 1).arg(l / 2.55, 0, 'f', 1);
    }
    
    QString color_to_ciexyz(const QColor &color)
    {
        const double r = color.redF();
        const double g = color.greenF();
        const double b = color.blueF();

        const double x = r * 0.4124564 + g * 0.3575761 + b * 0.1804375;
        const double y = r * 0.2126729 + g * 0.7151522 + b * 0.0721750;
        const double z = r * 0.0193339 + g * 0.1191920 + b * 0.9503041;
        
        return QString("xyz(%1, %2, %3)").arg(x * 100, 0, 'f', 2).arg(y * 100, 0, 'f', 2).arg(z * 100, 0, 'f', 2);
    }
    
    QString color_to_cielab(const QColor &color)
    {
        const double r = color.redF(),
                     g = color.greenF(),
                     b = color.blueF();

        const double x = r * 0.4124564 + g * 0.3575761 + b * 0.1804375,
                     y = r * 0.2126729 + g * 0.7151522 + b * 0.0721750,
                     z = r * 0.0193339 + g * 0.1191920 + b * 0.9503041;
        
        auto f = [](const double t)
        {
            return t > 0.008856 ? pow(t, 1.0/3.0) : (7.787 * t + 16.0/116.0);
        };
        
        const double l = 116.0 * f(y) - 16.0,
                     a = 500.0 * (f(x) - f(y)),
                     b_val = 200.0 * (f(y) - f(z));
        
        return QString("lab(%1, %2, %3)").arg(l, 0, 'f', 2).arg(a, 0, 'f', 2).arg(b_val, 0, 'f', 2);
    }
    
    QString color_to_oklab(const QColor &color)
    {
        const double r = color.redF();
        const double g = color.greenF();
        const double b = color.blueF();
        
        double l = 0.8189330101 * r + 0.3618667424 * g - 0.1288597137 * b;
        double m = 0.0329845436 * r + 0.9293118715 * g + 0.0361456387 * b;
        double s = 0.0482003018 * r + 0.2643662691 * g + 0.6338517070 * b;
        
        l = pow(l, 1.0/3.0);
        m = pow(m, 1.0/3.0);
        s = pow(s, 1.0/3.0);

        const double L = 0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s;
        const double a = 1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s;
        const double b_val = 0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s;
        
        return QString("oklab(%1, %2, %3)").arg(L, 0, 'f', 3).arg(a, 0, 'f', 3).arg(b_val, 0, 'f', 3);
    }
    
    QString color_to_oklch(const QColor &color) {
        const double r = color.redF();
        const double g = color.greenF();
        const double b = color.blueF();
        
        double l = 0.8189330101 * r + 0.3618667424 * g - 0.1288597137 * b;
        double m = 0.0329845436 * r + 0.9293118715 * g + 0.0361456387 * b;
        double s = 0.0482003018 * r + 0.2643662691 * g + 0.6338517070 * b;
        
        l = pow(l, 1.0/3.0);
        m = pow(m, 1.0/3.0);
        s = pow(s, 1.0/3.0);

        const double L = 0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s;
        const double a = 1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s;
        const double b_val = 0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s;
        
        double C = sqrt(a * a + b_val * b_val);
        double h = atan2(b_val, a) * 180.0 / M_PI;
        if (h < 0) h += 360.0;
        
        return QString("oklch(%1, %2, %3)").arg(L, 0, 'f', 3).arg(C, 0, 'f', 3).arg(h, 0, 'f', 2);
    }
    
    QString color_to_vec4(const QColor &color) {
        return QString("vec4(%1, %2, %3, %4)")
            .arg(color.redF(), 0, 'f', 3)
            .arg(color.greenF(), 0, 'f', 3)
            .arg(color.blueF(), 0, 'f', 3)
            .arg(color.alphaF(), 0, 'f', 3);
    }
    
    QString color_to_dec(const QColor &color) {
        return QString::number(color.rgb());
    }
    
    QString color_to_hex_int(const QColor &color) {
        return QString::number(color.rgb(), 16).toUpper();
    }
}

// ==================== Image Resizer ====================
namespace image_resizer {
    bool resize_image(const QString &source_path, const QString &output_path, const ResizeConfig &config) {
        const QImage image(source_path);
        if (image.isNull()) return false;
        
        QSize newSize(config.width, config.height);
        
        if (config.unit == Percent)
            newSize = image.size() * config.width / 100;
        
        QImage scaled;
        if (config.mode == Stretch)
            scaled = image.scaled(newSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        else if (config.mode == Fit)
            scaled = image.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        else
            scaled = image.scaled(newSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        const QString format = QFileInfo(output_path).suffix().toUpper();
        return scaled.save(output_path, format.toUtf8().constData());
    }
    
    bool batch_resize(const QStringList &source_paths, const QString &output_dir, const ResizeConfig &config) {
        bool allSuccess = true;
        for (const QString &path : source_paths) {
            QString fileName = QFileInfo(path).fileName();
            if (QString output_path = output_dir + "/" + fileName; !resize_image(path, output_path, config))
                allSuccess = false;
        }
        return allSuccess;
    }
    
    bool resize_with_fallback(const QString &source_path, const QString &output_path, 
                             const ResizeConfig &config, const QString &fallback_format) {
        if (resize_image(source_path, output_path, config))
            return true;
        
        // Fallback
        const QImage image(source_path);
        if (image.isNull()) return false;
        
        QSize newSize(config.width, config.height);
        if (config.unit == Percent)
            newSize = image.size() * config.width / 100;

        const QImage scaled = image.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        return scaled.save(output_path, fallback_format.toUtf8().constData());
    }
}

// ==================== Rename工具 ====================
namespace batch_rename {
    static QList<RenameResult> last_rename_results;
    
    QList<RenameResult> preview_rename(const QStringList &files, const QString &pattern) {
        QList<RenameResult> results;
        int counter = 1;
        
        for (const QString &file : files) {
            QFileInfo info(file);
            QString newName = pattern;
            newName.replace("{name}", info.baseName()),
            newName.replace("{ext}", info.suffix()),
            newName.replace("{num}", QString::number(counter)),
            newName.replace("{date}", QDateTime::currentDateTime().toString("yyyyMMdd"));
            
            RenameResult result;
            result.original = file,
            result.new_name = info.path() + "/" + newName + "." + info.suffix(),
            result.success = true,
            results.append(result),
            counter++;
        }
        
        return results;
    }
    
    QList<RenameResult> preview_replace(const QStringList &files, const QString &search, const QString &replace) {
        QList<RenameResult> results;
        
        for (const QString &file : files) {
            QFileInfo info(file);
            QString newName = info.baseName();
            newName.replace(search, replace);
            
            RenameResult result;
            result.original = file;
            result.new_name = info.path() + "/" + newName + "." + info.suffix();
            result.success = true;
            results.append(result);
        }
        
        return results;
    }
    
    QList<RenameResult> preview_regex_rename(const QStringList &files, const QString &pattern, const QString &replacement) {
        QList<RenameResult> results;
        const QRegularExpression regex(pattern);
        
        for (const QString &file : files)
        {
            QFileInfo info(file);
            QString newName = info.baseName();
            newName.replace(regex, replacement);
            
            RenameResult result;
            result.original = file,
            result.new_name = info.path() + "/" + newName + "." + info.suffix(),
            result.success = true;
            results.append(result);
        }
        
        return results;
    }
    
    bool apply_rename(const QList<RenameResult> &results) {
        last_rename_results.clear();
        bool allSuccess = true;
        
        for (const RenameResult &result : results) {
            RenameResult actual = result;
            if (QFile::exists(result.new_name))
                actual.success = false,
                actual.error = "File already exists",
                allSuccess = false;
             else if (!QFile::rename(result.original, result.new_name))
                actual.success = false,
                actual.error = "Rename failed",
                allSuccess = false;
            last_rename_results.append(actual);
        }
        
        return allSuccess;
    }
    
    bool undo_rename() {
        if (last_rename_results.isEmpty()) return false;
        
        bool allSuccess = true;
        for (const RenameResult &result : last_rename_results)
            if (result.success && !QFile::rename(result.new_name, result.original))
                allSuccess = false;

        last_rename_results.clear();
        return allSuccess;
    }
}

// ==================== 进制转换器 ====================
namespace base_converter {
    static const QString DIGITS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    
    qlonglong to_decimal(const QString &number, int base) {
        qlonglong result = 0;
        for (const QChar &c : number) {
            const int digit = DIGITS.indexOf(c);
            if (digit < 0 || digit >= base) return -1;
            result = result * base + digit;
        }
        return result;
    }
    
    QString from_decimal(qlonglong decimal, int base) {
        if (decimal == 0) return "0";
        QString result;
        while (decimal > 0) {
            result.prepend(DIGITS[decimal % base]);
            decimal /= base;
        }
        return result;
    }
    
    QString convert_base(const QString &number, const int from_base, const int to_base) {
        if (number.isEmpty() || from_base < 2 || to_base < 2) return {};

        const qlonglong decimal = to_decimal(number, from_base);
        if (decimal < 0) return {};
        
        return from_decimal(decimal, to_base);
    }
    
    bool is_valid_number(const QString &number, const int base) {
        if (number.isEmpty() || base < 2 || base > 60) return false;
        for (const QChar &c : number)
            if (const int digit = DIGITS.indexOf(c); digit < 0 || digit >= base) return false;
        return true;
    }
    
    int get_max_base() {
        return 60;
    }
}

// ==================== 时间戳转换 ====================
namespace timestamp_converter {
    QString timestamp_to_datetime(const qint64 timestamp, const QString &timezone) {
        QDateTime dt;
        if (timezone == "UTC" || timezone == "GMT")
            dt = QDateTime::fromSecsSinceEpoch(timestamp, QTimeZone::utc());
         else if (timezone == "Local")
            dt = QDateTime::fromSecsSinceEpoch(timestamp, QTimeZone::systemTimeZone());
         else
            dt = QDateTime::fromSecsSinceEpoch(timestamp, QTimeZone(timezone.toUtf8()));
        return dt.toString("yyyy-MM-dd hh:mm:ss");
    }

    qint64 datetime_to_timestamp(const QString &datetime, const QString &timezone) {
        QDateTime dt = QDateTime::fromString(datetime, "yyyy-MM-dd hh:mm:ss");
        if (!dt.isValid()) return 0;

        if (timezone == "UTC" || timezone == "GMT")
            dt.setTimeZone(QTimeZone::utc());
         else if (timezone != "Local")
            dt.setTimeZone(QTimeZone(timezone.toUtf8()));
        return dt.toSecsSinceEpoch();
    }

    QStringList get_available_timezones() {
        auto zones = QTimeZone::availableTimeZoneIds();
        QStringList result;
        result << "UTC" << "Local";
        for (const QByteArray &zone : zones)
            result << QString::fromUtf8(zone);
        return result;
    }

    qint64 get_current_timestamp() {return QDateTime::currentSecsSinceEpoch();}
    
    qint64 components_to_timestamp(int year, int month, int day, int hour, int minute, int second, const QString &timezone) {
        QDate date(year, month, day);
        QTime time(hour, minute, second);
        
        if (!date.isValid() || !time.isValid()) {
            return 0;
        }
        
        QDateTime dt(date, time);
        
        if (timezone == "UTC" || timezone == "GMT")
            dt.setTimeZone(QTimeZone::utc());
        else if (timezone != "Local")
            dt.setTimeZone(QTimeZone(timezone.toUtf8()));
        
        return dt.toSecsSinceEpoch();
    }
}

// ==================== Base64 ====================
namespace base64_tool {
    QString encode(const QString &text) {return QString::fromUtf8(text.toUtf8().toBase64());}
    
    QString decode(const QString &base64_text)
    {
        const QByteArray decoded = QByteArray::fromBase64(base64_text.toUtf8());
        QString result = QString::fromUtf8(decoded);
        if (result.contains(QChar::ReplacementCharacter) || result.isEmpty())
            result = QString::fromLatin1(decoded);
        return result;
    }
    
    QString encode_bytes(const QByteArray &data) {return QString::fromUtf8(data.toBase64());}
    
    QByteArray decode_to_bytes(const QString &base64_text) {return QByteArray::fromBase64(base64_text.toUtf8());}
    
    bool is_valid_base64(const QString &text) {
        static const QRegularExpression base64_regex("^[A-Za-z0-9+/]*={0,2}$");
        return base64_regex.match(text).hasMatch();
    }
}


// ==================== 文本比对 ====================
namespace text_diff {
    DiffResult compare(const QString &text1, const QString &text2) {
        DiffResult result;
        result.original = text1,
        result.modified = text2;
        
        // Simple line-by-line comparison
        QStringList lines1 = text1.split('\n'),
                    lines2 = text2.split('\n');
        
        int i = 0, j = 0;
        while (i < lines1.size() || j < lines2.size()) {
            if (i < lines1.size() && j < lines2.size() && lines1[i] == lines2[j])
                // Lines match
                i++,
                j++;
             else if (i < lines1.size() && (j >= lines2.size() || !lines2.contains(lines1[i])))
                // Line remove
                result.diff_ranges.append(qMakePair(-i - 1, -i - 1)),
                i++;
             else if (j < lines2.size() && (i >= lines1.size() || !lines1.contains(lines2[j])))
                // Line added
                result.diff_ranges.append(qMakePair(j, j)),
                j++;
             else {
                // Both changed
                if (i < lines1.size())
                    result.diff_ranges.append(qMakePair(-i - 1, -i - 1)),
                    i++;

                if (j < lines2.size())
                    result.diff_ranges.append(qMakePair(j, j)),
                    j++;
            }
        }
        
        return result;
    }
    
    QString generate_diff_html(const DiffResult &result) {
        QString html = "<!DOCTYPE html><html><head><style>"
                      ".del { background-color: #ffe6e6; color: #d32f2f; }"
                      ".ins { background-color: #e6ffe6; color: #388e3c; }"
                      ".line { font-family: monospace; white-space: pre-wrap; padding: 2px 4px; }"
                      "</style></head><body>";

        QStringList lines = result.original.split('\n');
        QSet<int> removedLines;

        for (const auto &key: result.diff_ranges | std::views::keys)
            if (key < 0)
                removedLines.insert(-key - 1);

        for (int i = 0; i < lines.size(); i++) {
            if (removedLines.contains(i))
                html += QString("<div class=\"line del\">- %1</div>").arg(lines[i].toHtmlEscaped());
             else
                html += QString("<div class=\"line\">  %1</div>").arg(lines[i].toHtmlEscaped());

        }

        html += "</body></html>";
        return html;
    }
}

// ==================== GZip ====================
namespace gzip_tool {
    QByteArray x_compress(const QByteArray &data) {
        if (data.isEmpty()) return {};
        
        z_stream zs;
        memset(&zs, 0, sizeof(zs));
        
        if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            return {};
        }
        
        zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
        zs.avail_in = static_cast<uInt>(data.size());
        
        int ret;
        char outbuffer[32768];
        QByteArray outstring;
        
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);
            
            ret = deflate(&zs, Z_FINISH);
            
            if (outstring.size() < zs.total_out) {
                outstring.append(outbuffer, static_cast<int>(zs.total_out - outstring.size()));
            }
        } while (ret == Z_OK);
        
        deflateEnd(&zs);
        
        if (ret != Z_STREAM_END) {
            return {};
        }
        
        return outstring;
    }
    
    QByteArray decompress(const QByteArray &data) {
        if (data.isEmpty()) return {};
        
        z_stream zs;
        memset(&zs, 0, sizeof(zs));
        
        if (inflateInit2(&zs, 15 + 16) != Z_OK) {
            return {};
        }
        
        zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
        zs.avail_in = static_cast<uInt>(data.size());
        
        int ret;
        char outbuffer[32768];
        QByteArray outstring;
        
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);
            
            ret = inflate(&zs, 0);
            
            if (outstring.size() < zs.total_out) {
                outstring.append(outbuffer, static_cast<int>(zs.total_out - outstring.size()));
            }
        } while (ret == Z_OK);
        
        inflateEnd(&zs);
        
        if (ret != Z_STREAM_END) {
            return {};
        }
        
        return outstring;
    }
}

// ==================== URL编解码 ====================
namespace url_tool {
    QString encode(const QString &text)
    {
        return QString::fromUtf8(QUrl::toPercentEncoding(text));
    }

    QString decode(const QString &encoded_text)
    {
        return QUrl::fromPercentEncoding(encoded_text.toUtf8());
    }
}

// ==================== 哈希/校验和生成器 ====================
namespace hash_tool {
    QString md5(const QString &text)
    {
        return QString::fromUtf8(QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Md5).toHex());
    }
    
    QString sha1(const QString &text)
    {
        return QString::fromUtf8(QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha1).toHex());
    }
    
    QString sha256(const QString &text)
    {
        return QString::fromUtf8(QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha256).toHex());
    }
    
    QString sha512(const QString &text)
    {
        return QString::fromUtf8(QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha512).toHex());
    }
    
    QString x_crc32(const QString &text) {
        static quint32 crc_table[256];
        static bool table_initialized = false;
        
        if (!table_initialized) {
            for (int i = 0; i < 256; i++) {
                quint32 crc = i;
                for (int j = 0; j < 8; j++) {
                    crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
                }
                crc_table[i] = crc;
            }
            table_initialized = true;
        }
        
        quint32 crc = 0xFFFFFFFF;
        for (QByteArray data = text.toUtf8(); const char c : data)
            crc = (crc >> 8) ^ crc_table[(crc ^ static_cast<quint8>(c)) & 0xFF];
        crc ^= 0xFFFFFFFF;
        
        return QString::number(crc, 16).toUpper();
    }
    
    QString file_md5(const QString &file_path)
    {
        QFile file(file_path);
        if (!file.open(QIODevice::ReadOnly)) return {};
        
        QCryptographicHash hash(QCryptographicHash::Md5);
        while (!file.atEnd())
            hash.addData(file.read(8192));
        file.close();
        return QString::fromUtf8(hash.result().toHex());
    }
    
    QString file_sha256(const QString &file_path)
    {
        QFile file(file_path);
        if (!file.open(QIODevice::ReadOnly)) return {};
        
        QCryptographicHash hash(QCryptographicHash::Sha256);
        while (!file.atEnd())
            hash.addData(file.read(8192));
        file.close();
        
        return QString::fromUtf8(hash.result().toHex());
    }
}

// ==================== 密码生成器 ====================
namespace password_gen {
    QString generate_password(const PasswordConfig &config) {
        QString chars;
        if (config.use_uppercase) chars += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        if (config.use_lowercase) chars += "abcdefghijklmnopqrstuvwxyz";
        if (config.use_numbers) chars += "0123456789";
        if (config.use_symbols)
        {
            if (config.custom_symbols.isEmpty())
                chars += "!@#$%^&*()_+-=[]{}|;:,.<>?";
            else
                chars += config.custom_symbols;
        }
        
        if (chars.isEmpty()) return {};
        if (config.exclude_ambiguous) {
            chars.remove('I'); chars.remove('l'); chars.remove('1');
            chars.remove('O'); chars.remove('0');
        }
        
        QString password;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, chars.length() - 1);
        
        for (int i = 0; i < config.length; i++)
            password.append(chars[dis(gen)]);

        return password;
    }
    
    double calculate_entropy(const QString &password) {
        int pool_size = 0;
        bool has_upper = false, has_lower = false, has_digit = false, has_symbol = false;
        
        for (const QChar &c : password) {
            if (c.isUpper()) has_upper = true;
            else if (c.isLower()) has_lower = true;
            else if (c.isDigit()) has_digit = true;
            else has_symbol = true;
        }
        
        if (has_upper) pool_size += 26;
        if (has_lower) pool_size += 26;
        if (has_digit) pool_size += 10;
        if (has_symbol) pool_size += 32;
        if (!pool_size) return 0.0;
        
        return password.length() * log2(pool_size);
    }
    
    QString evaluate_strength(const double entropy) {
        if (entropy < 30) return "Very Weak";
        if (entropy < 50) return "Weak";
        if (entropy < 70) return "Fair";
        if (entropy < 90) return "Strong";
        return "Very Strong";
    }
}

namespace common_backend
{
    void save_config(const bool darkmode)
    {
        const auto dirPath = QStringLiteral("./config");
        const QString filePath = dirPath + QStringLiteral("/config.json");

        if (const QDir dir(dirPath); !dir.exists()) {
            if (!QDir().mkpath(dirPath)) {
                qWarning("common_backend::save_config: failed to create config directory '%s'", qPrintable(dirPath));
            }
        }

        QJsonObject config_obj;
        config_obj[QStringLiteral("darkmode")] = darkmode;

        const QJsonDocument doc(config_obj);
        const QByteArray out = doc.toJson(QJsonDocument::Compact);

        QSaveFile saveFile(filePath);
        if (!saveFile.open(QIODevice::WriteOnly))
        {
            qWarning("common_backend::save_config: failed to open '%s' for writing: %s", qPrintable(filePath), qPrintable(saveFile.errorString()));
            QFile fallback(filePath);
            if (!fallback.open(QIODevice::WriteOnly))
            {
                qWarning("common_backend::save_config: fallback QFile also failed to open '%s': %s", qPrintable(filePath), qPrintable(fallback.errorString()));
                return;
            }
            fallback.write(out);
            fallback.close();
            return;
        }

        if (saveFile.write(out) == -1) {
            qWarning("common_backend::save_config: write failed for '%s': %s", qPrintable(filePath), qPrintable(saveFile.errorString()));
            return;
        }

        if (!saveFile.commit()) {
            qWarning("common_backend::save_config: commit failed for '%s': %s", qPrintable(filePath), qPrintable(saveFile.errorString()));
            return;
        }
    }

    bool read_config()
    {
        const QString dirPath = QStringLiteral("./config");
        const QString filePath = dirPath + QStringLiteral("/config.json");

        QFile file(filePath);

        // If file doesn't exist, create default and try again
        if (!file.exists()) {
            save_config(false);
        }

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("common_backend::read_config: failed to open '%s' for reading: %s", qPrintable(filePath), qPrintable(file.errorString()));
            // Try recreate default config
            save_config(false);
            if (!file.open(QIODevice::ReadOnly)) {
                qWarning("common_backend::read_config: still failed to open '%s' after creating default", qPrintable(filePath));
                return false;
            }
        }

        const QByteArray data = file.readAll();
        file.close();

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (doc.isNull() || !doc.isObject()) {
            qWarning("common_backend::read_config: config file '%s' is invalid: %s", qPrintable(filePath), qPrintable(parseError.errorString()));

            // Attempt to remove invalid file and recreate default
            if (!QFile::remove(filePath)) {
                qWarning("common_backend::read_config: failed to remove invalid config '%s'", qPrintable(filePath));
            }

            save_config(false);

            // Try reading the recreated default
            QFile file2(filePath);
            if (!file2.open(QIODevice::ReadOnly)) {
                qWarning("common_backend::read_config: failed to open recreated default config '%s'", qPrintable(filePath));
                return false;
            }
            const QByteArray data2 = file2.readAll();
            file2.close();

            const QJsonDocument doc2 = QJsonDocument::fromJson(data2, &parseError);
            if (doc2.isNull() || !doc2.isObject()) {
                qWarning("common_backend::read_config: recreated default config is invalid for '%s'", qPrintable(filePath));
                return false;
            }

            return doc2.object().value(QStringLiteral("darkmode")).toBool(false);
        }

        return doc.object().value(QStringLiteral("darkmode")).toBool(false);
    }
}
