#include "http.hpp"
#include <stdexcept>
namespace http {

        size_t http_callback(void * contents, size_t size, size_t number, bytes_data* result) {
            size_t total_size = size * number;
            result->insert(result->end(), static_cast<uint8_t*>(contents), static_cast<uint8_t*>(contents) + total_size);
            return total_size;
        }
    bytes_data post_request(const std::string& url, const bytes_data& data) {
        CURL * curl = curl_easy_init();

        if(!curl) {
            throw std::runtime_error("curl_easy_init failed");
        }

        bytes_data buffer;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_callback);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
        
        //TLS
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        //timer
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
       CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK) {
            throw std::runtime_error((std::string("CURL ERROR: ") + curl_easy_strerror(res)));
        }
        return buffer;

    }

    bytes_data get_request(const std::string& url) {
             CURL * curl = curl_easy_init();

        if(!curl) {
            throw std::runtime_error("curl_easy_init failed");
        }

        bytes_data buffer;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_callback);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        
        //TLS
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        //timer
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK) {
            throw std::runtime_error((std::string("CURL ERROR: ") + curl_easy_strerror(res)));
        }
        return buffer;
    }
}