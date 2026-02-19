#ifndef LEARNING_CPP_MAIN_WINDOW_H
#define LEARNING_CPP_MAIN_WINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

#include "lib_e/switch_button.h"
#include "backend.h"

class main_window : public QMainWindow {
    Q_OBJECT
public:
    explicit main_window(QWidget *parent = nullptr);
    
private slots:
    void on_tool_selected(const QTreeWidgetItem *item);
    void on_search_text_changed(const QString &text);
    void toggle_sidebar() const;
    void show_settings() const;
    void show_home() const;
    void toggle_view_mode();
    
    // Base64 tool slots
    void on_base64_encode() const;
    void on_base64_decode() const;
    
    // Url crypt tool slots
    void url_crypt() const;
    void url_decode() const;

    // Color picker slots
    void on_color_picker_pick() const;
    
    // Image resizer slots
    void on_image_resize() const;
    void on_image_batch_resize() const;
    
    // Rename tool slots
    void on_rename_select_files() const;
    void on_rename_preview() const;
    void on_rename_apply() const;
    void on_rename_undo() const;
    
    // Base converter slots
    void on_base_convert() const;
    
    // Timestamp converter slots
    void on_timestamp_to_datetime() const;
    void on_datetime_to_timestamp() const;
    void on_timestamp_edit_changed() const;
    void on_timestamp_add_year() const;
    void on_timestamp_sub_year() const;
    void on_timestamp_add_month() const;
    void on_timestamp_sub_month() const;
    void on_timestamp_add_day() const;
    void on_timestamp_sub_day() const;
    void on_timestamp_add_hour() const;
    void on_timestamp_sub_hour() const;
    void on_timestamp_add_minute() const;
    void on_timestamp_sub_minute() const;
    void on_timestamp_add_second() const;
    void on_timestamp_sub_second() const;
    
    // Text compare slots
    void on_text_compare() const;
    
    // GZip slots
    void on_gzip_compress() const;
    void on_gzip_decompress() const;
    
    // Hash tool slots
    void on_hash_generate() const;
    void on_hash_select_file() const;
    
    // Password generator slots
    void on_password_generate() const;
    void on_password_copy() const;

private:
    void setup_ui();
    void setup_sidebar();
    void setup_main_content();
    void create_tool_pages();
    void add_tool_category(const QString &category, const QStringList &tools);
    QTreeWidgetItem* find_tool_item(const QString &tool_name);

    QWidget* create_color_picker_page();
    QWidget* create_image_resizer_page();
    QWidget* create_rename_page();
    QWidget* create_base_converter_page();
    QWidget* create_timestamp_page();
    QWidget* create_base64_page();
    QWidget* create_text_compare_page();
    QWidget* create_gzip_page();
    QWidget* create_url_page();
    QWidget* create_hash_page();
    QWidget* create_password_page();
    QWidget* create_settings_page();
    static QWidget* create_home_page();
    QWidget* create_tool_page(const QString &tool_name);
    
    // UI Components
    QWidget *sidebar_widget_{};
    QTreeWidget *tool_tree_{};
    QLineEdit *search_box_{};
    QPushButton *toggle_sidebar_btn_{};
    QPushButton *settings_btn_{};
    QPushButton *home_btn_{};
    QStackedWidget *main_stack_{};
    QSplitter *splitter_{};

    // Tool categories and pages
    QMap<QString, QStringList> tool_categories_;
    QMap<QString, QWidget*> tool_pages_;
    QMap<QString, QTreeWidgetItem*> tool_items_;

    mutable QTreeWidgetItem *current_tool_item_ = nullptr;

    bool darkmode_;
    
    // Base64 tool components
    mutable QTextEdit *base64_input_{};
    mutable QTextEdit *base64_output_{};
    
    // Url crypt components
    mutable QTextEdit *Url_crypt_input_{};
    mutable QTextEdit *Url_crypt_output_{};
    
    // Color picker components
    mutable QPushButton *color_picker_btn_{};
    mutable QLabel *color_picker_label_{};
    mutable QComboBox *color_picker_combo_{};
    
    // Image resizer components
    mutable QSpinBox *image_width_spin_{};
    mutable QSpinBox *image_height_spin_{};
    mutable QComboBox *image_mode_combo_{};
    
    // Rename tool components
    mutable QPushButton *rename_select_btn_{};
    mutable QLineEdit *rename_pattern_{};
    mutable QPushButton *rename_preview_btn_{};
    mutable QPushButton *rename_apply_btn_{};
    mutable QPushButton *rename_undo_btn_{};
    mutable QTextEdit *rename_preview_text_{};
    mutable QStringList rename_selected_files_;
    
    // Base converter components
    mutable QSpinBox *base_from_spin_{};
    mutable QSpinBox *base_to_spin_{};
    mutable QLineEdit *base_input_{};
    mutable QLineEdit *base_output_{};
    
    // Timestamp converter components
    mutable QLineEdit *timestamp_input_{};
    mutable QComboBox *timestamp_timezone_{};
    mutable QLineEdit *timestamp_datetime_{};
    
    // Time editor components
    mutable QSpinBox *timestamp_year_{};
    mutable QSpinBox *timestamp_month_{};
    mutable QSpinBox *timestamp_day_{};
    mutable QSpinBox *timestamp_hour_{};
    mutable QSpinBox *timestamp_minute_{};
    mutable QSpinBox *timestamp_second_{};
    
    // Text compare components
    mutable QTextEdit *text_compare_original_{};
    mutable QTextEdit *text_compare_modified_{};
    mutable QTextEdit *text_compare_result_{};
    
    // Hash tool components
    mutable QTextEdit *hash_input_{};
    mutable QPushButton *hash_file_btn_{};
    mutable QComboBox *hash_algo_combo_{};
    mutable QLineEdit *hash_result_{};
    mutable QString hash_selected_file_;

    // gzip compress components
    mutable QTextEdit *gzip_compress_input{};
    mutable QTextEdit *gzip_compress_output{};
    
    // Password generator components
    mutable QSpinBox *password_length_{};
    mutable QCheckBox *password_upper_{};
    mutable QCheckBox *password_lower_{};
    mutable QCheckBox *password_number_{};
    mutable QCheckBox *password_symbol_{};
    mutable QCheckBox *password_ambiguous_{};
    mutable QLineEdit *password_result_{};
    mutable QLabel *password_strength_{};
    
    void set_style_() const;
};


#endif //LEARNING_CPP_MAIN_WINDOW_H
