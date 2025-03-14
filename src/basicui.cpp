#include "basicui.h"
//#include "ts.h"

    uint8_t BasicUI::lang = 0;        // default language for text resources
    bool BasicUI::isBackOn = true;    // is returning to main settings? default=true
#ifndef ESP8266     
    Task *BasicUI::_WiFiScan = nullptr;
#endif
/**
 * Define configuration variables and controls handlers
 * 
 * Variables has literal names and are kept within json-configuration file on flash
 * Control handlers are bound by literal name with a particular method. This method is invoked
 * by manipulating controls
 * 
 */
void BasicUI::add_sections(bool skipBack){ // is returning to main settings skipped?
    LOG(println, F("UI: Creating webui vars"));

    // variable for UI language (specific to basicui translations)
    embui.var_create(FPSTR(P_LANGUAGE), LANG::RU);

    lang = embui.param(FPSTR(P_LANGUAGE)).toInt();
    isBackOn = !skipBack;

    /**
     * обработчики действий
     */ 
    // вывод BasicUI секций
    embui.section_handle_add(FPSTR(T_SETTINGS), section_settings_frame);    // generate "settings" UI section
    embui.section_handle_add(FPSTR(T_SH_NETW), block_settings_netw);        // generate "network settings" UI section
    embui.section_handle_add(FPSTR(T_SH_TIME), block_settings_time);         // generate "time settings" UI section
    //embui.section_handle_add(FPSTR(T_SH_OTHER), show_settings_other);

    // обработка базовых настроек
    embui.section_handle_add(FPSTR(T_SET_WIFI), set_settings_wifi);         // обработка настроек WiFi Client
    embui.section_handle_add(FPSTR(T_SET_WIFIAP), set_settings_wifiAP);     // обработка настроек WiFi AP
    embui.section_handle_add(FPSTR(T_SET_MQTT), set_settings_mqtt);         // обработка настроек MQTT
    embui.section_handle_add(FPSTR(T_SET_SCAN), set_scan_wifi);             // обработка сканирования WiFi
    embui.section_handle_add(FPSTR(T_SET_TIME), set_settings_time);         // установки даты/времени
    embui.section_handle_add(FPSTR(P_LANGUAGE), set_language);              // смена языка интерфейса
    embui.section_handle_add(FPSTR(T_REBOOT), set_reboot);                  // перезагрузка

#ifdef EMBUI_USE_FTP
    embui.section_handle_add(FPSTR(T_SET_FTP), set_ftp);                    // обработка настроек FTP
    embui.section_handle_add(FPSTR(T_CHK_FTP), set_chk_ftp);                // обработка переключателя FTP
#endif
    //embui.section_handle_add(FPSTR(T_004B), set_settings_other);
}

/**
 * This code adds "Settings" section to the MENU
 * it is up to you to properly open/close Interface menu json_section
 */
void BasicUI::opt_setup(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->option(FPSTR(T_SETTINGS), FPSTR(T_DICT[lang][TD::D_SETTINGS]));     // пункт меню "настройки"
}

/**
 * формирование секции "настроек",
 * вызывается либо по выбору из "меню" либо при вызове из
 * других блоков/обработчиков
 * 
 */
void BasicUI::section_settings_frame(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface("");       // саму секцию целиком не обрабатываем

    interf->json_section_main(FPSTR(T_SETTINGS), FPSTR(T_DICT[lang][TD::D_SETTINGS]));

    interf->select(FPSTR(P_LANGUAGE), String(lang), String(FPSTR(T_DICT[lang][TD::D_LANG])), true);
    interf->option("0", F("Rus"));
    interf->option("1", F("Eng"));
    interf->json_section_end();

    interf->spacer();

    interf->button(FPSTR(T_SH_NETW), FPSTR(T_DICT[lang][TD::D_WIFI_MQTT]));  // кнопка перехода в настройки сети
    interf->button(FPSTR(T_SH_TIME), FPSTR(T_DICT[lang][TD::D_Time]));       // кнопка перехода в настройки времени

    // call for user_defined function that may add more elements to the "settings page"
    user_settings_frame(interf, data);

    interf->spacer();
    block_settings_update(interf, data);                                     // добавляем блок интерфейса "обновления ПО"

    interf->json_section_end();
    interf->json_frame_flush();
}

/**
 *  BasicUI блок интерфейса настроек WiFi/MQTT
 */
void BasicUI::block_settings_netw(Interface *interf, JsonObject *data){
    if (!interf) return;
    Task *_t = new Task(
        500,
        TASK_ONCE, [](){
            CALL_INTF_EMPTY(BasicUI::set_scan_wifi);
            TASK_RECYCLE; },
        &ts, false);
    _t->enableDelayed();
    interf->json_frame_interface();

    // Headline
    interf->json_section_main(FPSTR(T_OPT_NETW), FPSTR(T_DICT[lang][TD::D_WIFI_MQTT]));

    // форма настроек Wi-Fi Client
    interf->json_section_hidden(FPSTR(T_SET_WIFI), FPSTR(T_DICT[lang][TD::D_WiFiClient]));
    interf->spacer(FPSTR(T_DICT[lang][TD::D_WiFiClientOpts]));
    interf->text(FPSTR(P_hostname), FPSTR(T_DICT[lang][TD::D_Hostname]));
    interf->json_section_line();
    interf->select_edit(FPSTR(P_WCSSID), String(WiFi.SSID()), String(FPSTR(T_DICT[lang][TD::D_WiFiSSID])));
    interf->json_section_end();
    interf->button(FPSTR(T_SET_SCAN), FPSTR(T_DICT[lang][TD::D_Scan]), FPSTR(P_GREEN), 21);
    interf->json_section_end();
    interf->password(FPSTR(P_WCPASS), String(""), FPSTR(T_DICT[lang][TD::D_Password]));
    //interf->json_section_line();
    //interf->button(FPSTR(T_SET_SCAN), FPSTR(T_DICT[lang][TD::D_Scan]), FPSTR(P_GREEN));
    interf->button_submit(FPSTR(T_SET_WIFI), FPSTR(T_DICT[lang][TD::D_CONNECT]), FPSTR(P_GRAY));
    //interf->json_section_end();
    interf->json_section_end();

    // форма настроек Wi-Fi AP
    interf->json_section_hidden(FPSTR(T_SET_WIFIAP), FPSTR(T_DICT[lang][TD::D_WiFiAP]));
    interf->text(FPSTR(P_APhostname), embui.param(FPSTR(P_hostname)), String(FPSTR(T_DICT[lang][TD::D_Hostname])));
    interf->spacer(FPSTR(T_DICT[lang][TD::D_WiFiAPOpts]));
    interf->comment(FPSTR(T_DICT[lang][TD::D_MSG_APOnly]));
    interf->checkbox(FPSTR(P_APonly), FPSTR(T_DICT[lang][TD::D_APOnlyMode]));
    interf->password(FPSTR(P_APpwd),  FPSTR(T_DICT[lang][TD::D_MSG_APProtect]));
    interf->button_submit(FPSTR(T_SET_WIFIAP), FPSTR(T_DICT[lang][TD::D_SAVE]), FPSTR(P_GRAY));
    interf->json_section_end();

    // форма настроек MQTT
    interf->json_section_hidden(FPSTR(T_SET_MQTT), FPSTR(T_DICT[lang][TD::D_MQTT]));
    interf->text(FPSTR(P_m_host), FPSTR(T_DICT[lang][TD::D_MQTT_Host]));
    interf->number(FPSTR(P_m_port), FPSTR(T_DICT[lang][TD::D_MQTT_Port]));
    interf->text(FPSTR(P_m_user), FPSTR(T_DICT[lang][TD::D_User]));
    interf->password(FPSTR(P_m_pass), FPSTR(T_DICT[lang][TD::D_Password]));
    interf->text(FPSTR(P_m_pref), FPSTR(T_DICT[lang][TD::D_MQTT_Topic]));
    interf->number(FPSTR(P_m_tupd), FPSTR(T_DICT[lang][TD::D_MQTT_Interval]));
    interf->button_submit(FPSTR(T_SET_MQTT), FPSTR(T_DICT[lang][TD::D_CONNECT]), FPSTR(P_GRAY));
    interf->json_section_end();

#ifdef EMBUI_USE_FTP
    // форма настроек FTP
    interf->json_section_hidden("H", FPSTR(T_DICT[lang][TD::D_FTP]));
        interf->json_section_begin("C", "");
            interf->checkbox(FPSTR(T_CHK_FTP), String(embui.cfgData.isftp), FPSTR(T_DICT[lang][TD::D_FTP]), true);
        interf->json_section_end();
        interf->json_section_begin(FPSTR(T_SET_FTP), "");
            interf->text(FPSTR(P_ftpuser), FPSTR(T_DICT[lang][TD::D_User]));
            interf->password(FPSTR(P_ftppass), FPSTR(T_DICT[lang][TD::D_Password]));
            interf->button_submit(FPSTR(T_SET_FTP), FPSTR(T_DICT[lang][TD::D_SAVE]), FPSTR(P_GRAY));
        interf->json_section_end();
    interf->json_section_end();
#endif

    interf->spacer();
    interf->button(FPSTR(T_SETTINGS), FPSTR(T_DICT[lang][TD::D_EXIT]));

    interf->json_section_end();

    interf->json_frame_flush();
}

/**
 *  BasicUI блок загрузки обновлений ПО
 */
void BasicUI::block_settings_update(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_section_hidden(FPSTR(T_DO_OTAUPD), FPSTR(T_DICT[lang][TD::D_Update]));
    interf->spacer(FPSTR(T_DICT[lang][TD::D_FWLOAD]));
    interf->file(FPSTR(T_DO_OTAUPD), FPSTR(T_DO_OTAUPD), FPSTR(T_DICT[lang][TD::D_UPLOAD]));
    interf->button(FPSTR(T_REBOOT), FPSTR(T_DICT[lang][TD::D_REBOOT]),!data?String(FPSTR(P_RED)):String("")); // кнопка перезагрузки
}

/**
 *  BasicUI блок настройки даты/времени
 */
void BasicUI::block_settings_time(Interface *interf, JsonObject *data){
    if (!interf) return;
    interf->json_frame_interface();

    // Headline
    interf->json_section_main(FPSTR(T_SET_TIME), FPSTR(T_DICT[lang][TD::D_DATETIME]));

    interf->comment(FPSTR(T_DICT[lang][TD::D_MSG_TZSet01]));     // комментарий-описание секции

    // сперва рисуем простое поле с текущим значением правил временной зоны из конфига
    interf->text(FPSTR(P_TZSET), FPSTR(T_DICT[lang][TD::D_MSG_TZONE]));

    // user-defined NTP server
    interf->text(FPSTR(P_userntp), FPSTR(T_DICT[lang][TD::D_NTP_Secondary]));
    // manual date and time setup
    interf->comment(FPSTR(T_DICT[lang][TD::D_MSG_DATETIME]));
    interf->text(FPSTR(P_DTIME), String(""), "", false);
    interf->hidden(FPSTR(P_DEVICEDATETIME),""); // скрытое поле для получения времени с устройства
    interf->button_submit(FPSTR(T_SET_TIME), FPSTR(T_DICT[lang][TD::D_SAVE]), FPSTR(P_GRAY));

    interf->spacer();

    // exit button
    interf->button(FPSTR(T_SETTINGS), FPSTR(T_DICT[lang][TD::D_EXIT]));

    // close and send frame
    interf->json_section_end();
    interf->json_frame_flush();

    // формируем и отправляем кадр с запросом подгрузки внешнего ресурса со списком правил временных зон
    // полученные данные заместят предыдущее поле выпадающим списком с данными о всех временных зонах
    interf->json_frame_custom(F("xload"));
    interf->json_section_content();
                    //id            val                         label   direct  skipl URL for external data
    interf->select(FPSTR(P_TZSET), embui.param(FPSTR(P_TZSET)), "",     false,  true, F("/js/tz.json"));
    interf->json_section_end();
    interf->json_frame_flush();
}

/**
 * Обработчик настроек WiFi в режиме клиента
 */
void BasicUI::set_settings_wifi(Interface *interf, JsonObject *data){
    if (!data || embui.sysData.isWSConnect) return;

    SETPARAM(FPSTR(P_hostname));        // сохраняем hostname в конфиг

    String ssid = (*data)[FPSTR(P_WCSSID)];    // переменные доступа в конфиге не храним
    String pwd = (*data)[FPSTR(P_WCPASS)];     // фреймворк хранит последнюю доступную точку самостоятельно

    Task *t = new Task(300, TASK_ONCE, nullptr, &ts, false, nullptr, [ssid, pwd](){
        WiFi.disconnect();
        if(ssid){
            LOG(printf_P, PSTR("UI WiFi: Connecting to %s\n"), ssid.c_str());
            embui.var(FPSTR(P_APonly),"0"); // сборосим режим принудительного AP, при попытке подключения к роутеру
            embui.save();
            embui.wifi_connect(ssid.c_str(), pwd.c_str());
        } else {
            embui.wifi_connect();           // иницируем WiFi-подключение с новыми параметрами
            LOG(println, F("UI WiFi: No SSID defined!"));
        }
        TASK_RECYCLE;
    });
    t->enableDelayed();

    if(isBackOn) section_settings_frame(interf, data);            // переходим в раздел "настройки"
}

/**
 * Обработчик настроек WiFi в режиме AP
 */
void BasicUI::set_settings_wifiAP(Interface *interf, JsonObject *data){
    if (!data) return;

    if (data->containsKey(FPSTR(P_APhostname))){
        embui.var((FPSTR(P_hostname)), (*data)[FPSTR(P_APhostname)]); // для обоих режимов (STA-AP) одно и то же хранилище имени
    }
    //SETPARAM(FPSTR(P_hostname));    // эти переменные будут сохранены в конфиг-файл
    SETPARAM(FPSTR(P_APonly));
    SETPARAM(FPSTR(P_APpwd));

    embui.save();
    bool isAPmode = embui.param(FPSTR(P_APonly))=="1";
    if(isAPmode){
        embui.wifi_switchtoAP();
    } else {
        embui.wifi_connect(); 
    }
    if(isBackOn) section_settings_frame(interf, data);    // переходим в раздел "настройки"
}

/**
 * Обработчик настроек MQTT
 */
void BasicUI::set_settings_mqtt(Interface *interf, JsonObject *data){
    if (!data) return;
    // сохраняем настройки в конфиг
    SETPARAM(FPSTR(P_m_host));
    SETPARAM(FPSTR(P_m_port));
    SETPARAM(FPSTR(P_m_user));
    SETPARAM(FPSTR(P_m_pass));
    SETPARAM(FPSTR(P_m_pref));
    SETPARAM(FPSTR(P_m_tupd));
    //SETPARAM(FPSTR(P_m_tupd), some_mqtt_object.semqtt_int((*data)[FPSTR(P_m_tupd)]));

    embui.save();

    if(isBackOn) section_settings_frame(interf, data); 
}

/**
 * Обработчик сканирования WiFi
 */
void BasicUI::set_scan_wifi(Interface *interf, JsonObject *data){
    if (!interf) return;

    if (WiFi.scanComplete() == -2) {
        #ifdef ESP8266
        WiFi.scanNetworksAsync(scan_complete);     // Сканируем с коллбеком, по завершению скана запустится scan_complete()
        #endif
        #ifdef ESP32
        WiFi.scanNetworks(true);         // У ESP нет метода с коллбеком, поэтому просто сканируем
    }
    if (WiFi.scanComplete() < 0){
        if(!_WiFiScan){
            _WiFiScan = new Task(
                TASK_SECOND,
                TASK_ONCE, [](){
                    LOG(printf_P, PSTR("UI WiFi: Scan task running\n"));
                    if (WiFi.scanComplete() < 0){
                        ts.getCurrentTask()->restartDelayed();
                    }
                    if (WiFi.scanComplete() >= 0) {
                        scan_complete(WiFi.scanComplete());
                        TASK_RECYCLE; _WiFiScan = nullptr;
                    }
                },
                &ts, false);
            _WiFiScan->enableDelayed();
        } else {
            _WiFiScan->restartDelayed();
        }
        #endif
        LOG(printf_P, PSTR("UI WiFi: WiFi scan starting\n"));
    }
    if (WiFi.scanComplete()== -1) return;
    interf->json_frame_custom(F("xload"));
    interf->json_section_content();
    String ssid = WiFi.SSID();
    interf->select_edit(FPSTR(P_WCSSID), ssid, String(""), false, true);
    for (int i = 0; i < WiFi.scanComplete(); i++) {
        interf->option(WiFi.SSID(i), WiFi.SSID(i));
        LOG(printf_P, PSTR("UI WiFi: WiFi Net %s\n"), WiFi.SSID(i).c_str());
    }
    if(ssid.isEmpty())
        interf->option("", ""); // at the end of list
    interf->json_section_end();
    interf->json_section_end();
    interf->json_frame_flush();
    if (WiFi.scanComplete() >= 0) {
        WiFi.scanDelete();
        LOG(printf_P, PSTR("UI WiFi: Scan List deleted\n"));
    }
}

/**
 * Обработчик настроек даты/времени
 */
void BasicUI::set_settings_time(Interface *interf, JsonObject *data){
    if (!data) return;

    // Save and apply timezone rules
    String tzrule = (*data)[FPSTR(P_TZSET)];
    if (!tzrule.isEmpty()){
        SETPARAM(FPSTR(P_TZSET));
        embui.timeProcessor.tzsetup(tzrule.substring(4).c_str());   // cutoff '000_' prefix key
    }

    SETPARAM(FPSTR(P_userntp), embui.timeProcessor.setcustomntp((*data)[FPSTR(P_userntp)]));

    LOG(printf_P,PSTR("UI: devicedatetime=%s\n"),(*data)[FPSTR(P_DEVICEDATETIME)].as<String>().c_str());

    String datetime=(*data)[FPSTR(P_DTIME)];
    if (datetime.length())
        embui.timeProcessor.setTime(datetime);
    else if(!embui.sysData.wifi_sta) {
        datetime=(*data)[FPSTR(P_DEVICEDATETIME)].as<String>();
        if (datetime.length())
            embui.timeProcessor.setTime(datetime);
    }

    if(isBackOn) section_settings_frame(interf, data); 
}

void BasicUI::set_language(Interface *interf, JsonObject *data){
        if (!data) return;

    //String _l= (*data)[FPSTR(P_LANGUAGE)];
    SETPARAM(FPSTR(P_LANGUAGE), lang = (*data)[FPSTR(P_LANGUAGE)].as<unsigned short>(); );
    if(isBackOn) section_settings_frame(interf, data); 
}

void BasicUI::embuistatus(Interface *interf){
    if (!interf) return;
    interf->json_frame_value();
    interf->value(F("pTime"), embui.timeProcessor.getFormattedShortTime(), true);
    interf->value(F("pMem"), String(ESP.getFreeHeap()), true);
    interf->value(F("pUptime"), String(millis()/1000), true);
    interf->json_frame_flush();
}

#ifdef EMBUI_USE_FTP
void BasicUI::set_ftp(Interface *interf, JsonObject *data){
    if (!data) return;

    String user = (*data)[FPSTR(P_ftpuser)];
    String pass = (*data)[FPSTR(P_ftppass)];

    SETPARAM(FPSTR(P_ftpuser));
    SETPARAM(FPSTR(P_ftppass));

    BasicUI::set_chk_ftp(interf, data);

    //embui.ftpSrv.stop();
    //embui.ftpSrv.begin(user, pass);
    //embui.save();
    if(isBackOn) section_settings_frame(interf, data); 
}

void BasicUI::set_chk_ftp(Interface *interf, JsonObject *data){
    if (!data) return;

    if(data->containsKey(FPSTR(T_CHK_FTP))){
        embui.cfgData.isftp = (*data)[FPSTR(T_CHK_FTP)]=="1";
    }

    embui.var(FPSTR(P_cfgData), String(embui.cfgData.flags));

    if(embui.cfgData.isftp){
        embui.ftpSrv.stop();
        embui.ftpSrv.begin(embui.param(FPSTR(P_ftpuser)), embui.param(FPSTR(P_ftppass)));
    } else
        embui.ftpSrv.stop();
    //embui.save();
}
#endif

void BasicUI::set_reboot(Interface *interf, JsonObject *data){
    if (!data) return;
    bool isReboot = !embui.sysData.isWSConnect;
    if(isReboot){
        Task *t = new Task(TASK_SECOND*5, TASK_ONCE, nullptr, &ts, false, nullptr, [](){ LOG(println, F("Rebooting...")); delay(100); ESP.restart(); });
        t->enableDelayed();
    }
    if(interf){
        interf->json_frame_interface();
        block_settings_update(interf, isReboot?nullptr:data);
        interf->json_frame_flush();
    }
}

// stub function - переопределяется в пользовательском коде при необходимости добавить доп. пункты в меню настройки
void user_settings_frame(Interface *interf, JsonObject *data){};

// после завершения сканирования обновляем список WiFi
void BasicUI::scan_complete(int n){
    Interface *interf = embui.ws.count()? new Interface(&embui, &embui.ws, 512) : nullptr;
    set_scan_wifi(interf, nullptr);
    LOG(printf_P, PSTR("UI WiFi: Scan complete %d networks found\n"), n);
    delete interf;
}

void BasicUI::show_progress(Interface *interf, JsonObject *data){
    if (!interf) return;

    interf->json_frame_interface();
    interf->json_section_hidden(FPSTR(T_DO_OTAUPD), String(FPSTR(T_DICT[lang][TD::D_Update])) + String(F(" : ")) + (*data)[FPSTR(T_UPROGRESS)].as<String>()+ String("%"));
    interf->json_section_end();
    interf->json_frame_flush();
}

/*
 * OTA update progress
 */
uint8_t uploadProgress(size_t len, size_t total){
    DynamicJsonDocument doc(256);
    JsonObject obj = doc.to<JsonObject>();
    static int prev = 0;
    float part = total / 25.0;  // logger chunks
    int curr = len / part;
    uint8_t progress = 100*len/total;
    if (curr != prev) {
        prev = curr;
        LOG(printf_P, PSTR("%u%%.."), progress );
        obj[FPSTR(T_UPROGRESS)] = String(progress);
        CALL_INTF_OBJ(BasicUI::show_progress);
    }
    return progress;
}