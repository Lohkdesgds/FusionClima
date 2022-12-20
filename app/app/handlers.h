#pragma once
#define WIN32_LEAN_AND_MEAN

#include <Lunaris-Process/process.h>
#include <cpp-httplib/httplib.h>

#include <time.h>
#include <thread>
#include <future>
#include "tools.h"

#include "display.h"
#include "esp32_interface.h"

const std::string root_path_public = "./public_host";
const std::string page_not_found = "/error/404.html";
const std::string page_auth_failed = "/error/auth_failure.html";
const std::string page_host_failed = "/error/host_failure.html";

const std::string app_path_call = "python.exe";//"machine_learning/worker.exe";
const std::string app_file_read = "pred_history.txt";

const size_t split_point = 1 << 14;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
const int port = 553;
#else
const int port = 80;
#endif

void _internal_logger(const std::string&);
void static_logger(const httplib::Request& req, const httplib::Response& res);
void file_request_handler(const httplib::Request& req, httplib::Response& res);
httplib::Server::HandlerResponse pre_router_handler(const httplib::Request& req, httplib::Response& res, Display& dsp, ESP32_interface& esp);
void exception_handler(const httplib::Request& req, httplib::Response& res, std::exception_ptr ep);
void error_handler(const httplib::Request& req, httplib::Response& res);
//void post_routing_handler(const httplib::Request& req, httplib::Response& res);

// add temp, hum
void ai_add(const float, const float);
// get temp, hum, prev
//void ai_get(float&, float&, float&);
// lê do arquivo e pronto
void ai_get_latest(float& val);