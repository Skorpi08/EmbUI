// This framework originaly based on JeeUI2 lib used under MIT License Copyright (c) 2019 Marsel Akhkamov
// then re-written and named by (c) 2020 Anton Zolotarev (obliterator) (https://github.com/anton-zolotarev)
// also many thanks to Vortigont (https://github.com/vortigont), kDn (https://github.com/DmytroKorniienko)
// and others people

#include "ui.h"

/**
 * @brief - create generic html input element
 * @param id - element id
 * @param type - html element type, ex. 'text'
 * @param value - element value
 * @param label - element label
 * @param direct - if true, element value in send via ws on-change 
 */
void Interface::html_input(const String &id, const String &type, const String &value, const String &label, bool direct){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = FPSTR(P_input);
    obj[FPSTR(P_type)] = type;
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = value;
    obj[FPSTR(P_label)] = label;
    if (direct) obj[FPSTR(P_directly)] = true;

    frame_add_safe(obj.as<JsonObject>());
}

/**
 * @brief - create "number" html field with optional step, min, max constraints
 * Template accepts types suitable to be added to the ArduinoJson document used as a dictionary
 */
void Interface::number(const String &id, const String &label)
{
    number(id, embui->param(id), label, String(0), String(0), String(0));
}

void Interface::number(const String &id, const String &value, const String &label)
{
    number(id, value, label, String(0), String(0), String(0));
}

void Interface::number(const String &id, const String &label, const String &step, const String &min, const String &max)
{
    number(id, embui->param(id), label, step, min, max);
}

void Interface::number(const String &id, const String &value, const String &label, const String &step, const String &min, const String &max){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = FPSTR(P_input);
    obj[FPSTR(P_type)] = FPSTR(P_number);
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = value;
    obj[FPSTR(P_label)] = label;
    if (min) obj[FPSTR(P_min)] = min;
    if (max) obj[FPSTR(P_max)] = max;
    if (step) obj[FPSTR(P_step)] = step;

    frame_add_safe(obj.as<JsonObject>());
}

/**
 * @brief - create "range" html field with step, min, max constraints
 * Template accepts types suitable to be added to the ArduinoJson document used as a dictionary
 */
void Interface::range(const String &id, const String &min, const String &max, const String &step, const String &label, bool directly){
    range(id, embui->param(id), min, max, step, label, directly);
}

void Interface::range(const String &id, const String &value, const String &min, const String &max, const String &step, const String &label, bool directly){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = FPSTR(P_input);
    obj[FPSTR(P_type)] = F("range");
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = value;
    obj[FPSTR(P_label)] = label;
    if (directly) obj[FPSTR(P_directly)] = true;

    obj[FPSTR(P_min)] = min;
    obj[FPSTR(P_max)] = max;
    obj[FPSTR(P_step)] = step;

    frame_add_safe(obj.as<JsonObject>());
}

void Interface::select(const String &id, const String &value, const String &label, bool directly, bool skiplabel, const String &exturl, bool editable){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = F("select");
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = value;
    obj[FPSTR(P_label)] = skiplabel ? "" : label;
    if (directly) obj[FPSTR(P_directly)] = true;
    if (!exturl.isEmpty())
        obj[F("url")] = exturl;
    if (editable) obj[FPSTR(P_editable)] = true;

    if (!frame_add_safe(obj.as<JsonObject>()))
        return;

    section_stack.end()->idx--;
    json_section_begin(FPSTR(P_options), "", false, false, false, section_stack.end()->block.getElement(section_stack.end()->idx));
}

/**
 * @brief - create "display" div with custom css selector
 * could be used for making all kinds of "sensor" outputs on the page with live-updated values without the need to redraw interface element
 * @param id - element/div DOM id
 * @param value  - element value (treated as text)
 * @param class - base css class for Display, full css seletor created as "class id" to allow many sensors inherit base class
 * @param params - additional parameters (reserved for future use)
 */
void Interface::display(const String &id, const String &value, const String &css, const JsonObject &params){
    String cssclass(css);   // make css slector like "class id", id used as a secondary distinguisher 
    if (!css.length())
        cssclass += F("display");
    cssclass += F(" ");
    cssclass += id;
    custom(id, F("txt"), value, cssclass, params);
}

/**
 * @brief - Creates html element with image type
 * could be used for display image and animation from intertal or external storage
 * @param id elevent/div DOM id
 * @param value - element value (source destination, example - "/image/myimage.svg" or "https://example.com/image/myimage.jpg")
 */
void Interface::image(const String &id, const String &value){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = F("img");
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = value;
    frame_add_safe(obj.as<JsonObject>());
}

/**
 * @brief - Creates html element with cutomized type and arbitrary parameters
 * used to create user-defined interface elements with custom css/js handlers
 */
void Interface::custom(const String &id, const String &type, const String &value, const String &label, const JsonObject &param){
    StaticJsonDocument<IFACE_STA_JSON_SIZE*2> obj; // по этот контрол выделяем IFACE_STA_JSON_SIZE*2 т.к. он может быть большой...
    obj[FPSTR(P_html)] = F("custom");;
    obj[FPSTR(P_type)] = type;
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = value;
    obj[FPSTR(P_label)] = label;
    JsonObject nobj = obj.createNestedObject(String(F("param")));
    nobj.set(param);
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::frame(const String &id, const String &value){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = F("iframe");
    obj[FPSTR(P_type)] = F("frame");
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = value;
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::frame2(const String &id, const String &value){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = F("iframe2");;
    obj[FPSTR(P_type)] = F("frame");
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = value;
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::hidden(const String &id, const String &value){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = FPSTR(P_hidden);
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = value;
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::hidden(const String &id){
    hidden(id, embui->param(id));
}

void Interface::constant(const String &id, const String &value, const String &label){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = F("const");
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = value;
    obj[FPSTR(P_label)] = label;
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::constant(const String &id, const String &label){
    constant(id, embui->param(id), label);
}

void Interface::text(const String &id, const String &value, const String &label, bool directly){
    html_input(id, String(F("text")), value, label, directly);
}

void Interface::text(const String &id, const String &label, bool directly){
    text(id, embui->param(id), label, directly);
}

void Interface::time(const String &id, const String &value, const String &label){
    html_input(id, String(FPSTR(P_time)), value, label);
}

void Interface::time(const String &id, const String &label){
    time(id, embui->param(id), label);
}

void Interface::date(const String &id, const String &value, const String &label){
    html_input(id, String(FPSTR(P_date)), value, label);
}

void Interface::date(const String &id, const String &label){
    time(id, embui->param(id), label);
}

void Interface::datetime(const String &id, const String &value, const String &label){
    html_input(id, String(F("datetime-local")), value, label);
}

void Interface::datetime(const String &id, const String &label){
    datetime(id, embui->param(id), label);
}

void Interface::email(const String &id, const String &value, const String &label){
    html_input(id, String(F("email")), value, label);
}

void Interface::email(const String &id, const String &label){
    email(id, embui->param(id), label);
}

void Interface::password(const String &id, const String &value, const String &label){
    html_input(id, String(FPSTR(P_password)), value, label);
}

void Interface::password(const String &id, const String &label){
    password(id, embui->param(id), label);
}

void Interface::option(const String &value, const String &label){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_label)] = label;
    obj[FPSTR(P_value)] = value;
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::select(const String &id, const String &label, bool directly, bool skiplabel, bool editable){
    select(id, embui->param(id), label, directly, skiplabel);
}

void Interface::select_edit(const String &id, const String &value, const String &label, bool directly, bool skiplabel, const String &exturl, bool editable){
    select(id, value, label, directly, skiplabel, exturl, editable);
}

void Interface::select_edit(const String &id, const String &label, bool directly, bool skiplabel, bool editable){
    select(id, embui->param(id), label, directly, skiplabel, "", editable);
}


/**
 * элемент интерфейса checkbox
 * @param directly - значение чекбокса при изменении сразу передается на сервер без отправки формы
 * Template accepts types suitable to be added to the ArduinoJson document used as a dictionary
 */
void Interface::checkbox(const String &id, const String &value, const String &label, bool directly){
    html_input(id, String(F("checkbox")), value, label, directly);
}

void Interface::checkbox(const String &id, const String &label, bool directly){
    checkbox(id, embui->param(id), label, directly);
}

void Interface::color(const String &id, const String &value, const String &label){
    html_input(id, String(FPSTR(P_color)), value, label);
}

void Interface::color(const String &id, const String &label){
    color(id, embui->param(id), label);
}

void Interface::file(const String &name, const String &action, const String &label){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = FPSTR(P_file);
    obj[F("name")] = name;
    obj[F("action")] = action;
    obj[FPSTR(P_label)] = label;
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::button(const String &id, const String &label, const String &color, uint8_t top_margine){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = FPSTR(P_button);
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_color)] = color;
    obj[FPSTR(P_label)] = label;
    obj[FPSTR(P_top_margine)] = top_margine;
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::button_submit(const String &section, const String &label, const String &color, uint8_t top_margine){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = FPSTR(P_button);
    obj[FPSTR(P_submit)] = section;
    obj[FPSTR(P_color)] = color;
    obj[FPSTR(P_label)] = label;
    obj[FPSTR(P_top_margine)] = top_margine;
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::button_submit_value(const String &section, const String &value, const String &label, const String &color, uint8_t top_margine){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = FPSTR(P_button);
    obj[FPSTR(P_submit)] = section;
    obj[FPSTR(P_color)] = color;
    obj[FPSTR(P_label)] = label;
    obj[FPSTR(P_value)] = value;
    obj[FPSTR(P_top_margine)] = top_margine;
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::spacer(const String &label){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_html)] = F("spacer");
    if (label != "") obj[FPSTR(P_label)] = label;
    frame_add_safe(obj.as<JsonObject>());
}

void Interface::comment(const String &label){
    StaticJsonDocument<IFACE_STA_JSON_SIZE * 2> obj;
    obj[FPSTR(P_html)] = F("comment");
    if (label != "") obj[FPSTR(P_label)] = label;
    frame_add_safe(obj.as<JsonObject>());
}

/**
 * элемент интерфейса textarea
 * Template accepts types suitable to be added to the ArduinoJson document used as a dictionary
 */
void Interface::textarea(const String &id, const String &value, const String &label){
    html_input(id, String(F("textarea")), value, label);
}

void Interface::textarea(const String &id, const String &label){
    textarea(id, embui->param(id), label);
}

/**
 * @brief - Add 'value' object to the Interface frame
 * Template accepts types suitable to be added to the ArduinoJson document used as a dictionary
 */
void Interface::value(const String &id, const String &val, bool html){
    StaticJsonDocument<IFACE_STA_JSON_SIZE> obj;
    obj[FPSTR(P_id)] = id;
    obj[FPSTR(P_value)] = val;
    if (html) obj[FPSTR(P_html)] = true;

    frame_add_safe(obj.as<JsonObject>());
};

void Interface::value(const String &id, bool html){
    value(id, embui->param(id), html);
}

inline void Interface::value(JsonObjectConst data){
    frame_add_safe(data);
}

///////////////////////////////////////
void Interface::json_frame_value(){
    json[F("pkg")] = FPSTR(P_value);
    json[FPSTR(P_final)] = false;

    json_section_begin("root" + String(micros()));
}

void Interface::json_frame_interface(const String &name){
    json[F("pkg")] = F("interface");
    if (name != "") {
        json[F("app")] = name;
        json[F("mc")] = embui->mc;
        json[F("ver")] = F(TOSTRING(EMBUIVER));
    }
    json[FPSTR(P_final)] = false;

    json_section_begin("root" + String(micros()));
}

bool Interface::json_frame_add(JsonObjectConst obj) {
    if (!obj.memoryUsage()) // пустышки не передаем
        return false;

    LOG(printf_P, PSTR("UI: Frame add obj %u b, storage %d/%d"), obj.memoryUsage(), json.memoryUsage(), json.capacity());

    if (json.capacity() - json.memoryUsage() > obj.memoryUsage() + 16 && section_stack.end()->block.add(obj)) {
        section_stack.end()->idx++;
        LOG(printf_P, PSTR("...OK [%u]\tMEM: %u\n"), section_stack.end()->idx, ESP.getFreeHeap());
        return true;
    }
    LOG(printf_P, PSTR(" - Frame full! Heap: %u\n"), ESP.getFreeHeap());

    json_frame_send();
    json_frame_next();
    return false;
}

void Interface::json_frame_next(){
    json.clear();
    JsonObject obj = json.to<JsonObject>();
    for (int i = 0; i < section_stack.size(); i++) {
        if (i) obj = section_stack[i - 1]->block.createNestedObject();
        obj[FPSTR(P_section)] = section_stack[i]->name;
        obj[F("idx")] = section_stack[i]->idx;
        LOG(printf_P, PSTR("UI: section %u %s %u\n"), i, section_stack[i]->name.c_str(), section_stack[i]->idx);
        section_stack[i]->block = obj.createNestedArray(FPSTR(P_block));
    }
    LOG(printf_P, PSTR("json_frame_next: [%u], used %u, free %u\n"), section_stack.size(), obj.memoryUsage(), json.capacity() - json.memoryUsage());
}

void Interface::json_frame_clear(){
    while(section_stack.size()) {
        section_stack_t *section = section_stack.shift();
        delete section;
    }
    json.clear();
}

void Interface::json_frame_flush(){
    if (!section_stack.size()) return;
    LOG(println, F("json_frame_flush"));
    json[FPSTR(P_final)] = true;
    json_section_end();
    json_frame_send();
    json_frame_clear();
}

/**
 * @brief - begin custom UI secton
 * открывает секцию с указаным типом 'pkg', может быть обработан на клиенсткой стороне отлично от
 * интерфейсных пакетов 
 */
void Interface::json_frame_custom(const String &type){
    json[F("pkg")] = type;
    json[FPSTR(P_final)] = false;

    json_section_begin("root" + String(micros()));
}


void Interface::json_section_menu(){
    json_section_begin(FPSTR(P_menu));
}

void Interface::json_section_content(){
    json_section_begin(F("content"));
}

void Interface::json_section_line(const String &name){
    json_section_begin(name, "", false, false, true);
}

void Interface::json_section_main(const String &name, const String &label){
    json_section_begin(name, label, true);
}

void Interface::json_section_hidden(const String &name, const String &label){
    json_section_begin(name, label, false, true);
}

void Interface::json_section_begin(const String &name, const String &label, bool main, bool hidden, bool line){
    JsonObject obj;
    if (section_stack.size()) {
        obj = section_stack.end()->block.createNestedObject();
    } else {
        obj = json.as<JsonObject>();
    }
    json_section_begin(name, label, main, hidden, line, obj);
}

void Interface::json_section_begin(const String &name, const String &label, bool main, bool hidden, bool line, JsonObject obj){
    obj[FPSTR(P_section)] = name;
    if (label != "") obj[FPSTR(P_label)] = label;
    if (main) obj[F("main")] = true;
    if (hidden) obj[FPSTR(P_hidden)] = true;
    if (line) obj[F("line")] = true;

    section_stack_t *section = new section_stack_t;
    section->name = name;
    section->block = obj.createNestedArray(FPSTR(P_block));
    section->idx = 0;
    section_stack.add(section);
    LOG(printf_P, PSTR("UI: section begin %s [%u] %u free\n"), name.c_str(), section_stack.size(), json.capacity() - json.memoryUsage());
}

void Interface::json_section_end(){
    if (!section_stack.size()) return;

    section_stack_t *section = section_stack.pop();
    if (section_stack.size()) {
        section_stack.end()->idx++;
    }
    LOG(printf_P, PSTR("UI: section end %s [%u] MEM: %u\n"), section->name.c_str(), section_stack.size(), ESP.getFreeHeap());
    delete section;
}

/**
 * @brief - serialize and send json obj directly to the ws buffer
 */
void frameSendAll::send(const JsonObject& data){
    size_t length = measureJson(data);
    AsyncWebSocketMessageBuffer * buffer = ws->makeBuffer(length);
    if (!buffer)
        return;

    serializeJson(data, (char*)buffer->get(), ++length);
    ws->textAll(buffer);
};

/**
 * @brief - serialize and send json obj directly to the ws buffer
 */
void frameSendClient::send(const JsonObject& data){
    size_t length = measureJson(data);
    AsyncWebSocketMessageBuffer * buffer = cl->server()->makeBuffer(length);
    if (!buffer)
        return;

    serializeJson(data, (char*)buffer->get(), ++length);
    cl->text(buffer);
}

/**
 * @brief - add object to frame with mem overflow protection 
 */
bool Interface::frame_add_safe(const JsonObjectConst &jobj){
    size_t _cnt = FRAME_ADD_RETRY;

    do {
        --_cnt;
        #ifdef EMBUI_DEBUG
            if (!_cnt)
                LOG(println, FPSTR(P_ERR_obj2large));
        #endif
    } while (!json_frame_add(jobj) && _cnt );
    return _cnt>0;
}
