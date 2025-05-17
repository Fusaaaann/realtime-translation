/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <plugin-support.h>

#include <curl/curl.h>
#include <jansson.h>  // or another JSON parsing library

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("media_proc_plugin", "en-US")

// --- Helper Structure to accumulate API response data ---
struct api_response {
    char *data;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct api_response *mem = userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr)
        return 0;  // out of memory

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    return realsize;
}

// --- Function: Process frame via third-party API ---
/* This function takes an OBS frame, sends its data to an API, 
   waits for a response and then builds a new frame based on the response.
   (For example, the API might add an image filter or effect.)
*/
static obs_source_frame *process_frame_via_api(const obs_source_frame *frame)
{
    // 1. Extract frame data.
    // Depending on your needs, you might want to encode the frame->data
    // as an image (e.g., saving it to a memory buffer as PNG/JPEG)
    // For this example, we assume frame->data is already in a transferable format.

    // 2. Setup libcurl
    CURL *curl = curl_easy_init();
    if (!curl)
        return NULL; // handle error

    struct api_response response = {0};
    response.data = malloc(1);
    response.size = 0;

    // 3. Configure the request (adjust URL and parameters as needed)
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.example.com/process");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // Prepare HTTP headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 4. Set the request body to the frame data.
    // In a real scenario, you likely need to encode the frame (e.g., JPEG encode)
    // Here, we assume frame->data and frame->linesize define the raw bytes.
    // You might need to fill a buffer appropriately.
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, frame->data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)(frame->linesize[0] * frame->height));

    // 5. Set the write callback to capture the API response.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    // 6. Perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        blog(LOG_ERROR, "Curl error: %s", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(response.data);
        return NULL;
    }

    // Clean up curl resources
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // 7. Parse the API response
    // Assume the API returns JSON with a field "processed_data" containing image data encoded in base64,
    // or it could return a direct binary blob.
    json_error_t json_error;
    json_t *root = json_loads(response.data, 0, &json_error);
    free(response.data);
    if (!root) {
        blog(LOG_ERROR, "JSON parse error: %s", json_error.text);
        return NULL;
    }
    // For demonstration, assume the processed image data is returned directly in a field:
    json_t *data_j = json_object_get(root, "processed_data");
    if (!json_is_string(data_j)) {
        blog(LOG_ERROR, "Invalid JSON format: no processed_data string");
        json_decref(root);
        return NULL;
    }
    const char *processed_data = json_string_value(data_j);
    // In a real case, you would decode the base64 string and convert it into a raw image buffer.
    // For now, let’s assume this decoded raw data is directly usable.

    json_decref(root);

    // 8. Create a new OBS frame and populate with processed data
    // This is simplified. Typically, you would allocate an obs_source_frame structure,
    // allocate its data buffers, and copy your processed image bytes into it.
    obs_source_frame *new_frame = obs_source_frame_create(OBS_FOURCC_DEFAULT,
                                                          frame->width,
                                                          frame->height);
    if (!new_frame)
        return NULL;

    // Here you would decode processed_data from base64 (if necessary) and copy the contents
    // into new_frame->data buffers. For the demo, we just copy the original data.
    size_t data_size = frame->linesize[0] * frame->height;
    memcpy(new_frame->data[0], frame->data[0], data_size);
    new_frame->linesize[0] = frame->linesize[0];
    new_frame->color_range = frame->color_range;
    new_frame->color_matrix = frame->color_matrix;
    new_frame->color_primaries = frame->color_primaries;
    new_frame->timestamp = frame->timestamp;

    return new_frame;
}



// // --- Define the source information structure ---
// static struct obs_source_info media_proc_source_info = {
//     .id = "media_proc_source", // unique id
//     .type = OBS_SOURCE_TYPE_INPUT,
//     .output_flags = OBS_SOURCE_VIDEO,
//     .create = media_proc_create,
//     .destroy = media_proc_destroy,
//     .video_render = media_proc_video_render,
//     // You may include additional callbacks, such as .update or .get_properties, as needed.
// };

// --- OBS Module Load ---
bool obs_module_load(void)
{
    // Ensure libcurl is initialized prior to using it.
    curl_global_init(CURL_GLOBAL_DEFAULT);
    obs_register_source(&media_proc_source_info);
    blog(LOG_INFO, "Media processing plugin loaded successfully");
    return true;
}


void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
