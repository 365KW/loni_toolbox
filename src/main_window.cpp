#include "main_window.h"
#include "lib_e/switch_button.h"
#include "backend.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QScrollArea>
#include <QCheckBox>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QString>
#include <QStyleFactory>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QColorDialog>
#include <QDate>
#include <QStyleHints>
#include <QCursor>
#include <QMouseEvent>

main_window::main_window(QWidget *parent) : QMainWindow(parent)
{
    darkmode_ = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    setup_ui();
    set_style_();
}

void main_window::setup_ui()
{
    setWindowTitle("Loni Toolbox");
    resize(1200, 800);

    auto *central_widget = new QWidget(this);
    central_widget->setObjectName("CentralWidget");
    setCentralWidget(central_widget);

    auto *main_layout = new QHBoxLayout(central_widget);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    splitter_ = new QSplitter(Qt::Horizontal, this);
    splitter_->setHandleWidth(1);
    main_layout->addWidget(splitter_);

    setup_sidebar();
    setup_main_content();

    splitter_->addWidget(sidebar_widget_),
    splitter_->addWidget(main_stack_);

    splitter_->setCollapsible(0, false),
    splitter_->setStretchFactor(0, 0),
    splitter_->setStretchFactor(1, 1),
    splitter_->setSizes({260, 940});
}

void main_window::setup_sidebar() {
    sidebar_widget_ = new QWidget(this);
    sidebar_widget_->setObjectName("SidebarWidget");

    auto *sidebar_layout = new QVBoxLayout(sidebar_widget_);
    sidebar_layout->setSpacing(8);
    sidebar_layout->setContentsMargins(12, 16, 12, 16);

    search_box_ = new QLineEdit(this);
    search_box_->setObjectName("SearchBox"),
    search_box_->setPlaceholderText("搜索工具..."),
    search_box_->setClearButtonEnabled(true);
    connect(search_box_, &QLineEdit::textChanged, this, &main_window::on_search_text_changed);
    sidebar_layout->addWidget(search_box_);

    home_btn_ = new QPushButton("主页",this);
    home_btn_->setCursor(Qt::PointingHandCursor);
    connect(home_btn_, &QPushButton::clicked, this, &main_window::show_home);
    sidebar_layout->addWidget(home_btn_);

    tool_tree_ = new QTreeWidget(this);
    tool_tree_->setHeaderHidden(true);
    tool_tree_->setIndentation(10);
    tool_tree_->setRootIsDecorated(true);
    tool_tree_->setAnimated(true);
    tool_tree_->setFocusPolicy(Qt::NoFocus);
    connect(tool_tree_, &QTreeWidget::itemClicked, this, &main_window::on_tool_selected);
    sidebar_layout->addWidget(tool_tree_);

    settings_btn_ = new QPushButton("设置", this);
    settings_btn_->setCursor(Qt::PointingHandCursor);
    connect(settings_btn_, &QPushButton::clicked, this, &main_window::show_settings);
    sidebar_layout->addWidget(settings_btn_);

    add_tool_category("系统工具", {"取色板"});
    add_tool_category("图像工具", {"Image Resizer"});
    add_tool_category("文件工具", {"Rename工具"});
    add_tool_category("开发工具", {"进制转换器", "时间戳转换", "Base64解密","文本比对", "GZip", "url解密", "哈希/校验和生成器", "密码生成器"});

    tool_tree_->expandAll();
}

void main_window::add_tool_category(const QString &category, const QStringList &tools)
{
    auto *category_item = new QTreeWidgetItem(tool_tree_);
    category_item->setText(0, category);

    QFont font = category_item->font(0);
    font.setBold(true);
    category_item->setFont(0, font);
    category_item->setFlags(category_item->flags() & ~Qt::ItemIsSelectable);

    for (const auto &tool : tools) {
        auto *tool_item = new QTreeWidgetItem(category_item);
        tool_item->setText(0, tool);
        tool_items_[tool] = tool_item;
    }
}

QTreeWidgetItem * main_window::find_tool_item(const QString &tool_name)
{
    if (tool_items_.contains(tool_name))
        return tool_items_[tool_name];
    return nullptr;
}

void main_window::setup_main_content()
{
    main_stack_ = new QStackedWidget(this);

    auto *home_page = create_home_page();
    main_stack_->addWidget(home_page);

    create_tool_pages();

    auto *settings_page = create_settings_page();
    main_stack_->addWidget(settings_page);

    main_stack_->setCurrentIndex(0);
}

void main_window::create_tool_pages()
{
    QStringList all_tools = {"取色板", "Image Resizer", "Rename工具",
        "进制转换器", "时间戳转换", "Base64解密", "html转义",
        "文本比对", "GZip", "url解密", "哈希/校验和生成器", "密码生成器"
    };

    for (const auto &tool : all_tools) {
        QWidget *page = create_tool_page(tool);
        tool_pages_[tool] = page;
        main_stack_->addWidget(page);
    }
}

// ===== 辅助函数：创建页面框架 =====
static QWidget* create_page_container(const QString &title)
{
    auto *page_container = new QWidget();
    auto *page_layout = new QVBoxLayout(page_container);
    page_layout->setSpacing(20),
    page_layout->setContentsMargins(24, 24, 24, 24);

    auto *title_label = new QLabel(title, page_container);
    title_label->setObjectName("PageTitle");
    page_layout->addWidget(title_label);

    auto *scroll = new QScrollArea(page_container);
    scroll->setWidgetResizable(true),
    scroll->setFrameShape(QFrame::NoFrame);

    auto *card_widget = new QWidget();
    card_widget->setObjectName("ContentCard");

    auto *content_layout = new QVBoxLayout(card_widget);
    content_layout->setSpacing(16),
    content_layout->setContentsMargins(24, 24, 24, 24);

    page_container->setProperty("contentLayout", QVariant::fromValue(static_cast<void*>(content_layout))),
    page_container->setProperty("cardWidget", QVariant::fromValue(static_cast<void*>(card_widget))),
    page_container->setProperty("scrollArea", QVariant::fromValue(static_cast<void*>(scroll)));

    return page_container;
}

static QVBoxLayout* get_content_layout(const QWidget *page)
{
    return static_cast<QVBoxLayout*>(page->property("contentLayout").value<void*>());
}

static void finalize_page(const QWidget *page)
{
    auto *content_layout = get_content_layout(page);
    content_layout->addStretch();

    auto *scroll = static_cast<QScrollArea*>(page->property("scrollArea").value<void*>());
    auto *card_widget = static_cast<QWidget*>(page->property("cardWidget").value<void*>());
    scroll->setWidget(card_widget);

    auto *page_layout = qobject_cast<QVBoxLayout*>(page->layout());
    page_layout->addWidget(scroll);
}
///<summary>Create Color picker page</summary>
QWidget* main_window::create_color_picker_page() {
    auto *page = create_page_container("取色板");
    auto *layout = get_content_layout(page);

    color_picker_btn_ = new QPushButton("颜色选择器", page);
    connect(color_picker_btn_, &QPushButton::clicked, this, &main_window::on_color_picker_pick);
    
    color_picker_label_ = new QLabel("当前颜色: #000000", page);
    color_picker_label_->setMinimumHeight(50);
    color_picker_combo_ = new QComboBox(page);
    color_picker_combo_->addItems({"HEX", "RGB", "HSL", "HSV", "HWB", "NCol",
                           "CIEXYZ", "CIELAB", "Oklab", "Oklch", "VEC4", "DEC", "HEX Int"});

    auto *label_format = new QLabel("格式:", page);
    layout->addWidget(color_picker_btn_);
    layout->addWidget(color_picker_label_);
    layout->addWidget(label_format);
    layout->addWidget(color_picker_combo_);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_image_resizer_page() {
    auto *page = create_page_container("Image Resizer");
    auto *layout = get_content_layout(page);

    image_width_spin_ = new QSpinBox(page);
    image_width_spin_->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    image_width_spin_->setSuffix(" px");
    image_width_spin_->setRange(1, 10000);
    image_width_spin_->setValue(800);
    
    image_height_spin_ = new QSpinBox(page);
    image_height_spin_->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    image_height_spin_->setSuffix(" px");
    image_height_spin_->setRange(1, 10000);
    image_height_spin_->setValue(600);
    
    image_mode_combo_ = new QComboBox(page);
    image_mode_combo_->addItems({"填充 (Fill)", "适应 (Fit)", "拉伸 (Stretch)"});
    
    auto *btn_resize = new QPushButton("调整大小", page);
    connect(btn_resize, &QPushButton::clicked, this, &main_window::on_image_resize);
    
    auto *btn_batch = new QPushButton("批量调整", page);
    connect(btn_batch, &QPushButton::clicked, this, &main_window::on_image_batch_resize);

    auto *label_width = new QLabel("宽度:", page);
    auto *label_height = new QLabel("高度:", page);
    auto *label_mode = new QLabel("模式:", page);
    layout->addWidget(label_width);
    layout->addWidget(image_width_spin_);
    layout->addWidget(label_height);
    layout->addWidget(image_height_spin_);
    layout->addWidget(label_mode);
    layout->addWidget(image_mode_combo_);
    layout->addWidget(btn_resize);
    layout->addWidget(btn_batch);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_rename_page() {
    auto *page = create_page_container("Rename工具");
    auto *layout = get_content_layout(page);

    rename_select_btn_ = new QPushButton("选择文件", page);
    connect(rename_select_btn_, &QPushButton::clicked, this, &main_window::on_rename_select_files);
    
    rename_pattern_ = new QLineEdit(page);
    rename_pattern_->setPlaceholderText("命名模式 (例如: file_{num})");
    
    rename_preview_btn_ = new QPushButton("预览", page);
    connect(rename_preview_btn_, &QPushButton::clicked, this, &main_window::on_rename_preview);
    
    rename_apply_btn_ = new QPushButton("应用", page);
    connect(rename_apply_btn_, &QPushButton::clicked, this, &main_window::on_rename_apply);
    
    rename_undo_btn_ = new QPushButton("撤销", page);
    connect(rename_undo_btn_, &QPushButton::clicked, this, &main_window::on_rename_undo);
    
    rename_preview_text_ = new QTextEdit(page);
    rename_preview_text_->setReadOnly(true);
    rename_preview_text_->setPlaceholderText("预览结果...");

    auto *label_pattern = new QLabel("模式:", page);
    layout->addWidget(rename_select_btn_);
    layout->addWidget(label_pattern);
    layout->addWidget(rename_pattern_);
    layout->addWidget(rename_preview_btn_);
    layout->addWidget(rename_preview_text_);

    auto *btn_layout = new QHBoxLayout();
    btn_layout->addWidget(rename_apply_btn_);
    btn_layout->addWidget(rename_undo_btn_);
    layout->addLayout(btn_layout);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_base_converter_page() {
    auto *page = create_page_container("进制转换器");
    auto *layout = get_content_layout(page);

    base_from_spin_ = new QSpinBox(page);
    base_from_spin_->setButtonSymbols(QAbstractSpinBox::PlusMinus),
    base_from_spin_->setRange(2, 60),
    base_from_spin_->setValue(10);
    
    base_to_spin_ = new QSpinBox(page);
    base_to_spin_->setButtonSymbols(QAbstractSpinBox::PlusMinus),
    base_to_spin_->setRange(2, 60),
    base_to_spin_->setValue(16);
    
    base_input_ = new QLineEdit(page);
    base_input_->setPlaceholderText("输入数字");
    
    base_output_ = new QLineEdit(page);
    base_output_->setReadOnly(true);
    base_output_->setPlaceholderText("转换结果");
    
    auto *btn_convert = new QPushButton("转换", page);
    connect(btn_convert, &QPushButton::clicked, this, &main_window::on_base_convert);

    auto *label_from_base = new QLabel("从进制:", page);
    auto *label_to_base = new QLabel("到进制:", page);
    auto *label_input = new QLabel("输入:", page);
    auto *label_result = new QLabel("结果:", page);
    layout->addWidget(label_from_base);
    layout->addWidget(base_from_spin_);
    layout->addWidget(label_to_base);
    layout->addWidget(base_to_spin_);
    layout->addWidget(label_input);
    layout->addWidget(base_input_);
    layout->addWidget(btn_convert);
    layout->addWidget(label_result);
    layout->addWidget(base_output_);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_timestamp_page() {
    auto *page = create_page_container("时间戳转换");
    auto *layout = get_content_layout(page);

    timestamp_input_ = new QLineEdit(page);
    timestamp_input_->setPlaceholderText("输入时间戳");
    timestamp_timezone_ = new QComboBox(page);
    timestamp_timezone_->addItems(timestamp_converter::get_available_timezones());
    timestamp_timezone_->setCurrentText("Local");
    timestamp_datetime_ = new QLineEdit(page);
    timestamp_datetime_->setReadOnly(false);
    timestamp_datetime_->setPlaceholderText("日期时间 (yyyy-MM-dd hh:mm:ss)");
    auto *btn_to_datetime = new QPushButton("时间戳转日期", page);
    connect(btn_to_datetime, &QPushButton::clicked, this, &main_window::on_timestamp_to_datetime);
    auto *btn_to_timestamp = new QPushButton("日期转时间戳", page);
    connect(btn_to_timestamp, &QPushButton::clicked, this, &main_window::on_datetime_to_timestamp);
    auto *label_timestamp = new QLabel("时间戳:", page);
    auto *label_timezone = new QLabel("时区:", page);
    auto *label_datetime = new QLabel("日期时间:", page);
    layout->addWidget(label_timestamp);
    layout->addWidget(timestamp_input_);
    layout->addWidget(label_timezone);
    layout->addWidget(timestamp_timezone_);
    layout->addWidget(btn_to_datetime);
    layout->addWidget(label_datetime);
    layout->addWidget(timestamp_datetime_);
    layout->addWidget(btn_to_timestamp);

    auto *label_year = new QLabel("年:", page);
    auto *label_month = new QLabel("月:", page);
    auto *label_day = new QLabel("日:", page);
    auto *date_layout = new QHBoxLayout();
    timestamp_year_ = new QSpinBox(page);
    timestamp_year_->setRange(1970, 2100);
    timestamp_year_->setValue(QDate::currentDate().year());
    timestamp_year_->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    connect(timestamp_year_, QOverload<int>::of(&QSpinBox::valueChanged), this, &main_window::on_timestamp_edit_changed);
    date_layout->addWidget(label_year);
    date_layout->addWidget(timestamp_year_);
    date_layout->addStretch();
    timestamp_month_ = new QSpinBox(page);
    timestamp_month_->setRange(1, 12);
    timestamp_month_->setValue(QDate::currentDate().month());
    timestamp_month_->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    connect(timestamp_month_, QOverload<int>::of(&QSpinBox::valueChanged), this, &main_window::on_timestamp_edit_changed);
    date_layout->addWidget(label_month);
    date_layout->addWidget(timestamp_month_);
    date_layout->addStretch();
    timestamp_day_ = new QSpinBox(page);
    timestamp_day_->setRange(1, 31);
    timestamp_day_->setValue(QDate::currentDate().day());
    timestamp_day_->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    connect(timestamp_day_, QOverload<int>::of(&QSpinBox::valueChanged), this, &main_window::on_timestamp_edit_changed);
    date_layout->addWidget(label_day);
    date_layout->addWidget(timestamp_day_);
    date_layout->addStretch();
    layout->addLayout(date_layout);
    
    auto *label_hour = new QLabel("时:", page);
    auto *label_minute = new QLabel("分:", page);
    auto *label_second = new QLabel("秒:", page);
    auto *time_layout = new QHBoxLayout();

    timestamp_hour_ = new QSpinBox(page);
    timestamp_hour_->setRange(0, 23);
    timestamp_hour_->setValue(QTime::currentTime().hour());
    timestamp_hour_->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    connect(timestamp_hour_, QOverload<int>::of(&QSpinBox::valueChanged), this, &main_window::on_timestamp_edit_changed);
    time_layout->addWidget(label_hour);
    time_layout->addWidget(timestamp_hour_);
    time_layout->addStretch();
    timestamp_minute_ = new QSpinBox(page);
    timestamp_minute_->setRange(0, 59);
    timestamp_minute_->setValue(QTime::currentTime().minute());
    timestamp_minute_->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    connect(timestamp_minute_, QOverload<int>::of(&QSpinBox::valueChanged), this, &main_window::on_timestamp_edit_changed);
    time_layout->addWidget(label_minute);
    time_layout->addWidget(timestamp_minute_);
    time_layout->addStretch();
    timestamp_second_ = new QSpinBox(page);
    timestamp_second_->setRange(0, 59);
    timestamp_second_->setValue(QTime::currentTime().second());
    timestamp_second_->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    connect(timestamp_second_, QOverload<int>::of(&QSpinBox::valueChanged), this, &main_window::on_timestamp_edit_changed);
    time_layout->addWidget(label_second);
    time_layout->addWidget(timestamp_second_);
    time_layout->addStretch();
    layout->addLayout(time_layout);

    auto *btn_update_from_editor = new QPushButton("更新时间戳", page);
    connect(btn_update_from_editor, &QPushButton::clicked, this, &main_window::on_timestamp_edit_changed);
    layout->addWidget(btn_update_from_editor);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_base64_page() {
    auto *page = create_page_container("Base64解密");
    auto *layout = get_content_layout(page);

    base64_input_ = new QTextEdit(page);
    base64_input_->setPlaceholderText("输入文本...");
    base64_output_ = new QTextEdit(page);
    base64_output_->setReadOnly(true);
    base64_output_->setPlaceholderText("结果...");
    auto *btn_encode = new QPushButton("编码", page);
    auto *btn_decode = new QPushButton("解码", page);
    connect(btn_encode, &QPushButton::clicked, this, &main_window::on_base64_encode);
    connect(btn_decode, &QPushButton::clicked, this, &main_window::on_base64_decode);

    auto *label_input = new QLabel("输入:", page);
    auto *label_output = new QLabel("输出:", page);
    layout->addWidget(label_input);
    layout->addWidget(base64_input_);

    auto *btn_layout = new QHBoxLayout();
    btn_layout->addWidget(btn_encode);
    btn_layout->addWidget(btn_decode);
    layout->addLayout(btn_layout);

    layout->addWidget(label_output);
    layout->addWidget(base64_output_);

    finalize_page(page);
    return page;
}


QWidget* main_window::create_text_compare_page() {
    auto *page = create_page_container("文本比对");
    auto *layout = get_content_layout(page);

    text_compare_original_ = new QTextEdit(page);
    text_compare_original_->setPlaceholderText("原始文本...");
    
    text_compare_modified_ = new QTextEdit(page);
    text_compare_modified_->setPlaceholderText("修改后文本...");
    
    auto *btn_compare = new QPushButton("比对", page);
    connect(btn_compare, &QPushButton::clicked, this, &main_window::on_text_compare);
    
    text_compare_result_ = new QTextEdit(page);
    text_compare_result_->setReadOnly(true);
    text_compare_result_->setPlaceholderText("比对结果...");

    auto *label_original = new QLabel("原始文本:", page);
    auto *label_modified = new QLabel("修改后文本:", page);
    auto *label_result = new QLabel("结果:", page);
    layout->addWidget(label_original);
    layout->addWidget(text_compare_original_);
    layout->addWidget(label_modified);
    layout->addWidget(text_compare_modified_);
    layout->addWidget(btn_compare);
    layout->addWidget(label_result);
    layout->addWidget(text_compare_result_);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_gzip_page()
{
    auto *page = create_page_container("Gzip压缩");
    auto *layout = get_content_layout(page);

    gzip_compress_input = new QTextEdit(page);
    gzip_compress_input->setPlaceholderText("输入文本...");
    gzip_compress_output = new QTextEdit(page);
    gzip_compress_output->setReadOnly(true);
    gzip_compress_output->setPlaceholderText("结果...");
    auto *btn_compress = new QPushButton("压缩", page);
    auto *btn_decompress = new QPushButton("解压", page);
    connect(btn_compress, &QPushButton::clicked, this, &main_window::on_gzip_compress);
    connect(btn_decompress, &QPushButton::clicked, this, &main_window::on_gzip_decompress);

    auto *label_input = new QLabel("输入:", page);
    auto *label_output = new QLabel("输出:", page);
    layout->addWidget(label_input);
    layout->addWidget(gzip_compress_input);

    auto *btn_layout = new QHBoxLayout();
    btn_layout->addWidget(btn_compress);
    btn_layout->addWidget(btn_decompress);
    layout->addLayout(btn_layout);

    layout->addWidget(label_output);
    layout->addWidget(gzip_compress_output);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_url_page()
{
    auto *page = create_page_container("url编码/解码");
    auto *layout = get_content_layout(page);

    Url_crypt_input_ = new QTextEdit(page);
    Url_crypt_input_->setPlaceholderText("输入URL...");
    Url_crypt_output_ = new QTextEdit(page);
    Url_crypt_output_->setReadOnly(true);
    Url_crypt_output_->setPlaceholderText("结果...");
    auto *btn_encode = new QPushButton("编码", page);
    connect(btn_encode,&QPushButton::clicked,this,&main_window::url_crypt);
    auto *btn_decode = new QPushButton("解码", page);
    connect(btn_decode,&QPushButton::clicked,this,&main_window::url_decode);

    auto *label_input = new QLabel("输入:", page);
    auto *label_output = new QLabel("输出:", page);
    layout->addWidget(label_input);
    layout->addWidget(Url_crypt_input_);

    auto *btn_layout1 = new QHBoxLayout();
    btn_layout1->addWidget(btn_encode);
    btn_layout1->addWidget(btn_decode);
    layout->addLayout(btn_layout1);

    auto *btn_layout2 = new QHBoxLayout();
    layout->addLayout(btn_layout2);

    layout->addWidget(label_output);
    layout->addWidget(Url_crypt_output_);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_hash_page()
{
    auto *page = create_page_container("哈希/校验和生成器");
    auto *layout = get_content_layout(page);

    hash_input_ = new QTextEdit(page);
    hash_input_->setPlaceholderText("输入文本...");
    
    hash_file_btn_ = new QPushButton("选择文件", page);
    connect(hash_file_btn_, &QPushButton::clicked, this, &main_window::on_hash_select_file);
    
    hash_algo_combo_ = new QComboBox(page);
    hash_algo_combo_->addItems({"MD5", "SHA1", "SHA256", "SHA512", "CRC32"});
    
    hash_result_ = new QLineEdit(page);
    hash_result_->setReadOnly(true);
    hash_result_->setPlaceholderText("哈希值...");
    
    auto *btn_generate = new QPushButton("生成", page);
    connect(btn_generate, &QPushButton::clicked, this, &main_window::on_hash_generate);

    auto *label_input = new QLabel("输入文本:", page);
    auto *label_algo = new QLabel("算法:", page);
    auto *label_result = new QLabel("结果:", page);
    layout->addWidget(label_input);
    layout->addWidget(hash_input_);
    layout->addWidget(hash_file_btn_);
    layout->addWidget(label_algo);
    layout->addWidget(hash_algo_combo_);
    layout->addWidget(btn_generate);
    layout->addWidget(label_result);
    layout->addWidget(hash_result_);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_password_page()
{
    auto *page = create_page_container("密码生成器");
    auto *layout = get_content_layout(page);

    password_length_ = new QSpinBox(page);
    password_length_->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    password_length_->setRange(4, 128);
    password_length_->setValue(16);
    
    password_upper_ = new QCheckBox("包含大写字母 (A-Z)", page);
    password_upper_->setChecked(true);
    
    password_lower_ = new QCheckBox("包含小写字母 (a-z)", page);
    password_lower_->setChecked(true);
    
    password_number_ = new QCheckBox("包含数字 (0-9)", page);
    password_number_->setChecked(true);
    
    password_symbol_ = new QCheckBox("包含符号", page);
    password_symbol_->setChecked(true);
    
    password_ambiguous_ = new QCheckBox("排除易混淆字符", page);
    
    password_result_ = new QLineEdit(page);
    password_result_->setReadOnly(true);
    password_result_->setPlaceholderText("生成的密码...");
    
    password_strength_ = new QLabel("密码强度: ", page);
    
    auto *btn_generate = new QPushButton("生成密码", page);
    connect(btn_generate, &QPushButton::clicked, this, &main_window::on_password_generate);
    
    auto *btn_copy = new QPushButton("复制", page);
    connect(btn_copy, &QPushButton::clicked, this, &main_window::on_password_copy);

    auto *label_password_length = new QLabel("密码长度:", page);
    layout->addWidget(label_password_length),
    layout->addWidget(password_length_),
    layout->addWidget(password_upper_),
    layout->addWidget(password_lower_),
    layout->addWidget(password_number_),
    layout->addWidget(password_symbol_),
    layout->addWidget(password_ambiguous_);

    auto *btn_layout = new QHBoxLayout();
    btn_layout->addWidget(btn_generate),
    btn_layout->addWidget(btn_copy);
    layout->addLayout(btn_layout);

    layout->addWidget(password_result_),
    layout->addWidget(password_strength_);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_settings_page()
{
    auto *page = create_page_container("设置");
    auto *layout = get_content_layout(page);

    auto *switch_layout = new QHBoxLayout();
    auto *lbl_view_mode = new QLabel("暗色模式", page);
    dark_mode_switch_ = new SwitchButton(page);
    dark_mode_switch_->setChecked(darkmode_);
    switch_layout->addWidget(lbl_view_mode);
    switch_layout->addWidget(dark_mode_switch_);
    switch_layout->addStretch();

    connect(dark_mode_switch_, &SwitchButton::toggled, this, [this](bool) {
        toggle_view_mode();
    });

    auto *Common_setting_ = new QLabel("通用设置", page);
    QFont comm_sett_labed_font = Common_setting_->font();
    comm_sett_labed_font.setBold(true);
    Common_setting_->setFont(comm_sett_labed_font);
    layout->addWidget(Common_setting_);
    layout->addLayout(switch_layout);
    layout->addSpacing(20);

    finalize_page(page);
    return page;
}

QWidget* main_window::create_home_page() {
    auto *page = create_page_container("主页");
    auto *layout = get_content_layout(page);

    auto *lbl_welcome = new QLabel("欢迎使用 Loni Toolbox", page);
    QFont font = lbl_welcome->font();
    font.setPointSize(16);
    font.setBold(true);
    lbl_welcome->setFont(font);
    lbl_welcome->setAlignment(Qt::AlignCenter);

    auto *lbl_desc = new QLabel("请从左侧菜单选择工具", page);
    lbl_desc->setAlignment(Qt::AlignCenter);

    layout->addStretch();
    layout->addWidget(lbl_welcome);
    layout->addWidget(lbl_desc);
    layout->addStretch();

    finalize_page(page);
    return page;
}

QWidget* main_window::create_tool_page(const QString &tool_name)
{
    if (tool_name == "取色板")
        return create_color_picker_page();
    if (tool_name == "Image Resizer")
        return create_image_resizer_page();
    if (tool_name == "Rename工具")
        return create_rename_page();
    if (tool_name == "进制转换器")
        return create_base_converter_page();
    if (tool_name == "时间戳转换")
        return create_timestamp_page();
    if (tool_name == "Base64解密")
        return create_base64_page();
    if (tool_name == "文本比对")
        return create_text_compare_page();
    if (tool_name == "GZip")
        return create_gzip_page();
    if (tool_name == "url解密")
        return create_url_page();
    if (tool_name == "哈希/校验和生成器")
        return create_hash_page();
    if (tool_name == "密码生成器")
        return create_password_page();
    if (tool_name == "主页")
        return create_home_page();
    if (tool_name == "设置")
        return create_settings_page();
    return create_home_page();
}

void main_window::on_tool_selected(const QTreeWidgetItem *item)
{
    if (!item->parent())
    {
        if (auto *category = const_cast<QTreeWidgetItem*>(item); category->isExpanded())
            category->setExpanded(false);
        else
            category->setExpanded(true);
        if (current_tool_item_)
            tool_tree_->setCurrentItem(current_tool_item_);
        return;
    }

    current_tool_item_ = const_cast<QTreeWidgetItem*>(item);
    if (const QString tool_name = item->text(0); tool_pages_.contains(tool_name))
        main_stack_->setCurrentWidget(tool_pages_[tool_name]);
}

void main_window::on_search_text_changed(const QString &text)
{
    for (auto it = tool_items_.begin(); it != tool_items_.end(); ++it)
    {
        const bool match = text.isEmpty() || it.key().contains(text, Qt::CaseInsensitive);
        it.value()->setHidden(!match);
        if (match && it.value()->parent())
            it.value()->parent()->setHidden(false);
    }

    for (int i = 0; i < tool_tree_->topLevelItemCount(); ++i)
    {
        auto *category = tool_tree_->topLevelItem(i);
        bool has_visible = false;
        for (int j = 0; j < category->childCount(); ++j)
            if (!category->child(j)->isHidden())
            {
                has_visible = true;
                break;
            }
        category->setHidden(!has_visible);
    }
}

void main_window::toggle_sidebar() const{sidebar_widget_->setVisible(!sidebar_widget_->isVisible());}

void main_window::show_settings() const
{
    main_stack_->setCurrentIndex(main_stack_->count() - 1);
    current_tool_item_ = nullptr;
    tool_tree_->setCurrentItem(nullptr);
}

void main_window::show_home() const
{
    main_stack_->setCurrentIndex(0);
    current_tool_item_ = nullptr;
    tool_tree_->setCurrentItem(nullptr);
}

void main_window::toggle_view_mode()
{
    darkmode_ = !darkmode_;set_style_();
}

void main_window::set_style_() const {
    QString filename;
    if (darkmode_) {
        filename = "./asset/main_dark.qss";
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    }else {
        filename = "./asset/main_light.qss";
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
    }

    QFile qss(filename);
    QString stylesheet;
    if (qss.open(QFile::ReadOnly))
        stylesheet = qss.readAll();
    qss.close();
    qApp->setStyleSheet(stylesheet);
}

void main_window::on_base64_encode() const
{
    if (base64_input_ && base64_output_)
    {
        const QString input = base64_input_->toPlainText(),encoded = base64_tool::encode(input);
        base64_output_->setPlainText(encoded);
    }
}

void main_window::on_base64_decode() const
{
    if (base64_input_ && base64_output_)
    {
        const QString input = base64_input_->toPlainText(),decoded = base64_tool::decode(input);
        base64_output_->setPlainText(decoded);
    }
}

// ===== URL tool slots =====
void main_window::url_crypt() const
{
    if (Url_crypt_output_&&Url_crypt_input_)
    {
        const QString input = Url_crypt_input_->toPlainText(),res = url_tool::encode(input);
        Url_crypt_output_->setPlainText(res);
    }
}

void main_window::url_decode() const
{
    if (Url_crypt_output_&&Url_crypt_input_)
    {
        const QString input = Url_crypt_input_->toPlainText(),res = url_tool::decode(input);
        Url_crypt_output_->setPlainText(res);
    }
}

void main_window::on_color_picker_pick() const
{
    //TODO fix it
    QColorDialog colorDialog(nullptr);
    colorDialog.setCurrentColor(QColor(0, 0, 0));
    colorDialog.setOption(QColorDialog::ShowAlphaChannel);
    
    if (colorDialog.exec() == QDialog::Accepted) {
        const QColor color = colorDialog.currentColor();
        if (color.isValid() && color_picker_label_ && color_picker_combo_)
        {
            const QString format = color_picker_combo_->currentText();
            QString result;
            
            if (format == "RGB")
                result = color_picker::color_to_rgb(color);
            else if (format == "HSL")
                result = color_picker::color_to_hsl(color);
            else if (format == "HSV")
                result = color_picker::color_to_hsv(color);
            else if (format == "HWB")
                result = color_picker::color_to_hwb(color);
            else if (format == "NCol")
                result = color_picker::color_to_ncol(color);
            else if (format == "CIEXYZ")
                result = color_picker::color_to_ciexyz(color);
            else if (format == "CIELAB")
                result = color_picker::color_to_cielab(color);
            else if (format == "Oklab")
                result = color_picker::color_to_oklab(color);
            else if (format == "Oklch")
                result = color_picker::color_to_oklch(color);
            else if (format == "VEC4")
                result = color_picker::color_to_vec4(color);
            else if (format == "DEC")
                result = color_picker::color_to_dec(color);
            else if (format == "HEX Int")
                result = color_picker::color_to_hex_int(color);
            else
                result = color_picker::color_to_hex(color);
                
            color_picker_label_->setText("当前颜色: " + result);
            color_picker_label_->setStyleSheet("background-color: " + color.name() + "; color: " + (color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114 > 186 ? "black" : "white") + "; padding: 10px;");
        }
    }
}

void main_window::on_image_resize() const
{
    const QString source_path = QFileDialog::getOpenFileName(nullptr, "选择图片", QString(), "Images (*.png *.jpg *.jpeg *.bmp *.gif *.tiff)");
    if (source_path.isEmpty()) return;

    const QString output_path = QFileDialog::getSaveFileName(nullptr, "保存图片", QString(), "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)");
    if (output_path.isEmpty()) return;
    
    image_resizer::ResizeConfig config{};
    config.width = image_width_spin_->value();
    config.height = image_height_spin_->value();
    config.mode = static_cast<image_resizer::ResizeMode>(image_mode_combo_->currentIndex());
    config.unit = image_resizer::Pixel;
    config.keep_aspect = true;
    
    if (image_resizer::resize_image(source_path, output_path, config))
        QMessageBox::information(nullptr, "成功", "图片调整大小成功！");
    else
        QMessageBox::warning(nullptr, "错误", "图片调整大小失败！");
}

void main_window::on_image_batch_resize() const
{
    const QStringList source_paths = QFileDialog::getOpenFileNames(nullptr, "选择图片", QString(), "Images (*.png *.jpg *.jpeg *.bmp *.gif *.tiff)");
    if (source_paths.isEmpty()) return;

    const QString output_dir = QFileDialog::getExistingDirectory(nullptr, "选择输出目录");
    if (output_dir.isEmpty()) return;
    
    image_resizer::ResizeConfig config{};
    config.width = image_width_spin_->value();
    config.height = image_height_spin_->value();
    config.mode = static_cast<image_resizer::ResizeMode>(image_mode_combo_->currentIndex());
    config.unit = image_resizer::Pixel;
    config.keep_aspect = true;
    
    if (image_resizer::batch_resize(source_paths, output_dir, config))
        QMessageBox::information(nullptr, "成功", "批量调整大小成功！");
    else
        QMessageBox::warning(nullptr, "错误", "部分图片调整大小失败！");
}

// ===== Rename tool slots =====
void main_window::on_rename_select_files() const
{
    if (const QStringList files = QFileDialog::getOpenFileNames(nullptr, "选择文件", QString(), "All Files (*)"); !files.isEmpty()) {
        rename_selected_files_ = files;
        rename_select_btn_->setText(QString("已选择 %1 个文件").arg(files.size()));
    }
}

void main_window::on_rename_preview() const
{
    if (rename_selected_files_.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "请先选择文件！");
        return;
    }

    const QString pattern = rename_pattern_->text();
    if (pattern.isEmpty())
    {
        QMessageBox::warning(nullptr, "警告", "请输入命名模式！");
        return;
    }
    
    QList<batch_rename::RenameResult> results = batch_rename::preview_rename(rename_selected_files_, pattern);
    
    QString preview_text;
    for (const auto &result : results)
    {
        preview_text += QString("%1 -> %2 %3\n")
            .arg(result.original, result.new_name, result.success ? "✓" : "✗");
    }
    
    rename_preview_text_->setPlainText(preview_text);
}

void main_window::on_rename_apply() const
{
    if (rename_selected_files_.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "请先选择文件！");
        return;
    }

    const QString pattern = rename_pattern_->text();
    if (pattern.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "请输入命名模式！");
        return;
    }

    if (const QList<batch_rename::RenameResult> results = batch_rename::preview_rename(rename_selected_files_, pattern); batch_rename::apply_rename(results))
        QMessageBox::information(nullptr, "成功", "重命名成功！");
    else
        QMessageBox::warning(nullptr, "错误", "部分文件重命名失败！请检查文件占用等内容");
}

void main_window::on_rename_undo() const
{
    if (batch_rename::undo_rename())
        QMessageBox::information(nullptr, "成功", "撤销成功！");
    else
        QMessageBox::warning(nullptr, "错误", "撤销失败！");
}

// ===== Base converter slots =====
void main_window::on_base_convert() const
{
    const QString input = base_input_->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "请输入数字！");
        return;
    }

    const int from_base = base_from_spin_->value();
    const int to_base = base_to_spin_->value();
    
    if (!base_converter::is_valid_number(input, from_base)) {
        QMessageBox::warning(nullptr, "警告", "输入的数字与源进制不匹配！");
        return;
    }

    const QString result = base_converter::convert_base(input, from_base, to_base);
    base_output_->setText(result);
}

// ===== Timestamp converter slots =====
void main_window::on_timestamp_to_datetime() const
{
    QString timestamp_str = timestamp_input_->text().trimmed();
    if (timestamp_str.isEmpty()) {
        QMessageBox::warning(nullptr, "警告", "请输入正确时间戳！");
        return;
    }

    const qint64 timestamp = timestamp_str.toLongLong();
    QString timezone = timestamp_timezone_->currentText();

    const QString datetime = timestamp_converter::timestamp_to_datetime(timestamp, timezone);
    timestamp_datetime_->setText(datetime);
}

void main_window::on_datetime_to_timestamp() const
{
    const QString datetime_str = timestamp_datetime_->text().trimmed();
    if (datetime_str.isEmpty())
    {
        QMessageBox::warning(nullptr, "警告", "请输入日期时间！");
        return;
    }

    QString timezone = timestamp_timezone_->currentText();

    if (const qint64 timestamp = timestamp_converter::datetime_to_timestamp(datetime_str, timezone); timestamp > 0)
        timestamp_input_->setText(QString::number(timestamp));
    else
        QMessageBox::warning(nullptr, "错误", "无效的日期时间格式！请使用 yyyy-MM-dd hh:mm:ss");
}

// ===== Time Editor Slots =====
void main_window::on_timestamp_edit_changed() const
{
    const int year = timestamp_year_->value();
    const int month = timestamp_month_->value();
    const int day = timestamp_day_->value();
    const int hour = timestamp_hour_->value();
    const int minute = timestamp_minute_->value();
    const int second = timestamp_second_->value();
    
    QString timezone = timestamp_timezone_->currentText();

    if (const qint64 timestamp = timestamp_converter::components_to_timestamp(year, month, day, hour, minute, second, timezone); timestamp > 0)
    {
        timestamp_input_->setText(QString::number(timestamp));
        const QString datetime = timestamp_converter::timestamp_to_datetime(timestamp, timezone);
        timestamp_datetime_->setText(datetime);
    }
}

void main_window::on_timestamp_add_year() const
{
    timestamp_year_->setValue(timestamp_year_->value() + 1),
    on_timestamp_edit_changed();
}

void main_window::on_timestamp_sub_year() const
{
    timestamp_year_->setValue(timestamp_year_->value() - 1),
    on_timestamp_edit_changed();
}

void main_window::on_timestamp_add_month() const
{
    if (const int month = timestamp_month_->value(); month >= 12)
        timestamp_month_->setValue(1),
        timestamp_year_->setValue(timestamp_year_->value() + 1);
    else
        timestamp_month_->setValue(month + 1);
    on_timestamp_edit_changed();
}

void main_window::on_timestamp_sub_month() const
{
    if (const int month = timestamp_month_->value(); month <= 1)
        timestamp_month_->setValue(12),
        timestamp_year_->setValue(timestamp_year_->value() - 1);
    else
        timestamp_month_->setValue(month - 1);
    on_timestamp_edit_changed();
}

void main_window::on_timestamp_add_day() const
{
    timestamp_day_->setValue(timestamp_day_->value() + 1);
    on_timestamp_edit_changed();
}

void main_window::on_timestamp_sub_day() const
{
    timestamp_day_->setValue(timestamp_day_->value() - 1),
    on_timestamp_edit_changed();
}

void main_window::on_timestamp_add_hour() const
{
    if (const int hour = timestamp_hour_->value(); hour >= 23)
        timestamp_hour_->setValue(0),
        on_timestamp_add_day();
    else
        timestamp_hour_->setValue(hour + 1),
        on_timestamp_edit_changed();
}

void main_window::on_timestamp_sub_hour() const
{
    if (const int hour = timestamp_hour_->value(); hour <= 0)
        timestamp_hour_->setValue(23),
        on_timestamp_sub_day();
    else
        timestamp_hour_->setValue(hour - 1),
        on_timestamp_edit_changed();
}

void main_window::on_timestamp_add_minute() const
{
    if (const int minute = timestamp_minute_->value(); minute >= 59)
        timestamp_minute_->setValue(0),
        on_timestamp_add_hour();
    else
        timestamp_minute_->setValue(minute + 1),
        on_timestamp_edit_changed();

}

void main_window::on_timestamp_sub_minute() const
{
    if (const int minute = timestamp_minute_->value(); minute <= 0)
        timestamp_minute_->setValue(59),
        on_timestamp_sub_hour();
    else
        timestamp_minute_->setValue(minute - 1),
        on_timestamp_edit_changed();

}

void main_window::on_timestamp_add_second() const
{
    if (const int second = timestamp_second_->value(); second >= 59)
        timestamp_second_->setValue(0),
        on_timestamp_add_minute();
    else
        timestamp_second_->setValue(second + 1),
        on_timestamp_edit_changed();
}

void main_window::on_timestamp_sub_second() const
{
    if (const int second = timestamp_second_->value(); second <= 0)
        timestamp_second_->setValue(59),
        on_timestamp_sub_minute();
    else
        timestamp_second_->setValue(second - 1),
        on_timestamp_edit_changed();
}

// ===== Text compare slots =====
void main_window::on_text_compare() const
{
    const QString original = text_compare_original_->toPlainText();
    const QString modified = text_compare_modified_->toPlainText();
    
    text_diff::DiffResult result = text_diff::compare(original, modified);
    
    QString compare_result;
    QStringList lines1 = original.split('\n');
    QStringList lines2 = modified.split('\n');
    
    int i = 0, j = 0;
    while (i < lines1.size() || j < lines2.size())
    {
        if (i < lines1.size() && j < lines2.size() && lines1[i] == lines2[j])
            compare_result += QString("  %1\n").arg(lines1[i]),
            i++,
            j++;
        else if (i < lines1.size())
            compare_result += QString("- %1\n").arg(lines1[i]),
            i++;
        else if (j < lines2.size())
            compare_result += QString("+ %1\n").arg(lines2[j]),
            j++;
    }
    
    text_compare_result_->setPlainText(compare_result);
}

// ===== GZip slots =====
void main_window::on_gzip_compress() const
{
    const QByteArray input = gzip_compress_input->toPlainText().toUtf8();
    const QByteArray compressed = gzip_tool::x_compress(input);
    const QString result = base64_tool::encode_bytes(compressed);  // Base64 encode
    gzip_compress_output->setPlainText(result);
}

void main_window::on_gzip_decompress() const
{
    const QString input = gzip_compress_input->toPlainText();
    const QByteArray compressed = base64_tool::decode_to_bytes(input);  // Base64 decode
    const QByteArray result = gzip_tool::decompress(compressed);
    gzip_compress_output->setPlainText(QString::fromUtf8(result));
}

// ===== Hash tool slots =====
void main_window::on_hash_generate() const
{
    const QString input = hash_input_->toPlainText(),
                  algo = hash_algo_combo_->currentText();
    QString result;
    
    if (!input.isEmpty())
    {
        if (algo == "MD5")result = hash_tool::md5(input);
        else if (algo == "SHA1")result = hash_tool::sha1(input);
        else if (algo == "SHA256")result = hash_tool::sha256(input);
        else if (algo == "SHA512")result = hash_tool::sha512(input);
        else if (algo == "CRC32")result = hash_tool::x_crc32(input);
    }
    else if (!hash_selected_file_.isEmpty())
    {
        if (algo == "MD5" || algo == "SHA256") result = hash_tool::file_md5(hash_selected_file_);
        else QMessageBox::warning(nullptr, "警告", "文件只支持MD5和SHA256算法");
    }
    else
    {
        QMessageBox::warning(nullptr, "警告", "请输入文本或选择文件！");
        return;
    }
    
    hash_result_->setText(result);
}

void main_window::on_hash_select_file() const
{
    if (const QString file_path = QFileDialog::getOpenFileName(nullptr, "选择文件", QString(), "All Files (*)"); !file_path.isEmpty())
    {
        hash_selected_file_ = file_path;
        hash_input_->setPlaceholderText(QString("文件: %1").arg(file_path));
    }
}

// ===== Password generator slots =====
void main_window::on_password_generate() const
{
    password_gen::PasswordConfig config;
    config.length = password_length_->value(),
    config.use_uppercase = password_upper_->isChecked(),
    config.use_lowercase = password_lower_->isChecked(),
    config.use_numbers = password_number_->isChecked(),
    config.use_symbols = password_symbol_->isChecked(),
    config.exclude_ambiguous = password_ambiguous_->isChecked(),
    config.custom_symbols = "";

    if (const QString password = password_gen::generate_password(config); !password.isEmpty())
    {
        password_result_->setText(password);

        const double entropy = password_gen::calculate_entropy(password);
        const QString strength = password_gen::evaluate_strength(entropy);
        password_strength_->setText(QString("密码强度: %1 (熵: %2)").arg(strength).arg(entropy, 0, 'f', 1));
    }
}

void main_window::on_password_copy() const
{
    if (const QString password = password_result_->text(); !password.isEmpty())
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(password);
        QMessageBox::information(nullptr, "成功", "密码已复制到剪贴板！");
    }
}
