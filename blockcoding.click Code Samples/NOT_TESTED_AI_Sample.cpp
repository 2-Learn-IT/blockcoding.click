#include <Arduino.h>
#include <WiFi.h>
#include <esp32-hal-ledc.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "fd_forward.h"

const char* ssid     = "Blockcoding.click";
const char* password = "wlan4blockcoding.click";

#define PART_BOUNDARY "123456789000000000000987654321"

// This project was tested with the AI Thinker Model, M5STACK PSRAM Model and M5STACK WITHOUT PSRAM
#define CAMERA_MODEL_AI_THINKER
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WITHOUT_PSRAM

// Not tested with this model
//#define CAMERA_MODEL_WROVER_KIT

#if defined(CAMERA_MODEL_WROVER_KIT)
  #define PWDN_GPIO_NUM    -1
  #define RESET_GPIO_NUM   -1
  #define XCLK_GPIO_NUM    21
  #define SIOD_GPIO_NUM    26
  #define SIOC_GPIO_NUM    27
  
  #define Y9_GPIO_NUM      35
  #define Y8_GPIO_NUM      34
  #define Y7_GPIO_NUM      39
  #define Y6_GPIO_NUM      36
  #define Y5_GPIO_NUM      19
  #define Y4_GPIO_NUM      18
  #define Y3_GPIO_NUM       5
  #define Y2_GPIO_NUM       4
  #define VSYNC_GPIO_NUM   25
  #define HREF_GPIO_NUM    23
  #define PCLK_GPIO_NUM    22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_M5STACK_WITHOUT_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       17
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Camera model not selected"
#endif

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

void received_new_prediction(String prediction_class[], float prediction_probability[], int num_classes){
  for(int i=0; i < num_classes; i++){
      if(prediction_class[i]!=""){
        Serial.print("Class: ");
        Serial.print(prediction_class[i]);
        Serial.print(" Propability: ");
        Serial.printf("%f\n", prediction_probability[i]);
      }
  }
  Serial.println("");
}

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

static esp_err_t capture_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    
    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    size_t out_len, out_width, out_height;
    uint8_t * out_buf;
    bool s;
    if(fb->width > 400){
        if(fb->format == PIXFORMAT_JPEG){
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
        }
        esp_camera_fb_return(fb);
        return res;
    }

    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
    if (!image_matrix) {
        esp_camera_fb_return(fb);
        Serial.println("dl_matrix3du_alloc failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    out_buf = image_matrix->item;
    out_len = fb->width * fb->height * 3;
    out_width = fb->width;
    out_height = fb->height;

    s = fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
    esp_camera_fb_return(fb);
    if(!s){
        dl_matrix3du_free(image_matrix);
        Serial.println("to rgb888 failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    jpg_chunking_t jchunk = {req, 0};
    s = fmt2jpg_cb(out_buf, out_len, out_width, out_height, PIXFORMAT_RGB888, 90, jpg_encode_stream, &jchunk);
    dl_matrix3du_free(image_matrix);
    if(!s){
        Serial.println("JPEG compression failed");
        return ESP_FAIL;
    }
    return res;
}

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
  }
  return res;
}

static esp_err_t cmd_handler(httpd_req_t *req){
    char*  buf;
    size_t buf_len;
    sensor_t * s = esp_camera_sensor_get();
    int res = 0;
    String Feedback="";
    char cmd[128] = {0,};
    char variable1[256] = {0,};
    char value1[128] = {0,};
    char variable2[256] = {0,};
    char value2[128] = {0,};
    char variable3[256] = {0,};
    char value3[128] = {0,};
    
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
          httpd_query_key_value(buf, "cmd", cmd, sizeof(cmd));
          httpd_query_key_value(buf, "var1", variable1, sizeof(variable1));
          httpd_query_key_value(buf, "val1", value1, sizeof(value1));
          httpd_query_key_value(buf, "var2", variable2, sizeof(cmd));
          httpd_query_key_value(buf, "val2", value2, sizeof(value1));
          httpd_query_key_value(buf, "var3", variable3, sizeof(cmd));
          httpd_query_key_value(buf, "val3", value3, sizeof(value1));
        }
    }
    else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    if (buf)
      Feedback = String(buf);

    //Handling http://[...IP...]/control?cmd=xxx&val=xxx

    //Camera Control
    if(!strcmp(cmd, "quality")){
      int val = atoi(value1);  
      res = s->set_quality(s, val);
    } 
    else if(!strcmp(cmd, "contrast")){
      int val = atoi(value1);  
      res = s->set_contrast(s, val);
    } 
    else if(!strcmp(cmd, "brightness")){
      int val = atoi(value1);
      res = s->set_brightness(s, val);
    } 
    else if(!strcmp(cmd, "hmirror")){
      int val = atoi(value1);
      res = s->set_hmirror(s, val);
    } 
    else if(!strcmp(cmd, "vflip")){
      int val = atoi(value1);
      res = s->set_vflip(s, val);
    } 
    else if(!strcmp(cmd, "flash")) {
      int val = atoi(value1);
      ledcAttachPin(4, 4);  
      ledcSetup(4, 5000, 8);        
      ledcWrite(4,val);
    }

    //Receive TeachableMachine Results http://[...IP...]/control?cmd=TeachableMachineResults&var1=[name class 1]&val1=[propability class 1]&var2=[name class 2]&val2=[propability class 2]&var3=[name class 3]&val3=[propability class 3]
    else if (!strcmp(cmd, "TeachableMachineResults")) {
      String prediction_class[3] = {"","",""};
      float prediction_probability[3] = {0};

      prediction_class[0] = variable1;
      prediction_probability[0] = atof(value1);
      prediction_class[1] = variable2;
      prediction_probability[1] = atof(value2);
      prediction_class[2] = variable3;
      prediction_probability[2] = atof(value3);

      received_new_prediction(prediction_class, prediction_probability, 3);        
    }

    else if (!strcmp(cmd, "StartRobot")) {
        if(atoi(value1) == 1){
          //call Function to start Robot Move
          Serial.println("Robot Started!");
        }
        else if(atoi(value1) == 0){
          //call Function to stop Robot Move
          Serial.println("Robot Stopped!");
        }
    }

    //Other Commands
    else if (!strcmp(cmd, "mac")) {
      Feedback="MAC Address: "+WiFi.macAddress();
    }  
    else if (!strcmp(cmd, "restart")) {
      ESP.restart();
    }  
    else if (!strcmp(cmd, "digitalwrite")) {
      ledcDetachPin(atoi(variable1));
      pinMode(atoi(variable1), OUTPUT);
      digitalWrite(atoi(variable1), atoi(value1));
    }  

    /*else if (cmd=="your cmd") {
      // You can do anything
      // Feedback="<font color=\"red\">Hello World</font>";
    }*/
    else {
      res = -1;
    }
  
    if(res){
      return httpd_resp_send_500(req);
    }

    const char *resp = Feedback.c_str();
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, resp, strlen(resp));  
}


static const char PROGMEM INDEX_HTML[] = R"rawliteral(<!doctype html>
<html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width,initial-scale=1">
        <meta http-equiv="Access-Control-Allow-Headers" content="Origin, X-Requested-With, Content-Type, Accept">
        <meta http-equiv="Access-Control-Allow-Methods" content="GET,POST,PUT,DELETE,OPTIONS">
        <meta http-equiv="Access-Control-Allow-Origin" content="*">
        <title>AI Learning Lab</title>
        <style>
          body{font-family:Arial,Helvetica,sans-serif;background:#181818;color:#EFEFEF;font-size:16px}h2{font-size:18px}section.main{display:flex}#menu,section.main{flex-direction:column}#menu{display:flex;flex-wrap:nowrap;min-width:340px;background:#363636;padding:8px;border-radius:4px;margin-top:-10px;margin-right:10px}#content{display:flex;flex-wrap:wrap;align-items:stretch}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}figure img{display:block;width:100%;height:auto;border-radius:4px;margin-top:8px}@media (min-width: 800px) and (orientation:landscape){#content{display:flex;flex-wrap:nowrap;align-items:stretch}figure img{display:block;max-width:100%;max-height:calc(100vh - 40px);width:auto;height:auto}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}}section#buttons{display:flex;flex-wrap:nowrap;justify-content:space-between}#nav-toggle{cursor:pointer;display:block}#nav-toggle-cb{outline:0;opacity:0;width:0;height:0}#nav-toggle-cb:checked+#menu{display:none}.input-group{display:flex;flex-wrap:nowrap;line-height:22px;margin:5px 0}.input-group>label{display:inline-block;padding-right:10px;min-width:47%}.input-group input,.input-group select{flex-grow:1}.range-max,.range-min{display:inline-block;padding:0 5px}button{display:block;margin:5px;padding:0 12px;border:0;line-height:28px;cursor:pointer;color:#fff;background:#ff3034;border-radius:5px;font-size:16px;outline:0}button:hover{background:#ff494d}button:active{background:#f21c21}button.disabled{cursor:default;background:#a0a0a0}input[type=range]{-webkit-appearance:none;width:100%;height:22px;background:#363636;cursor:pointer;margin:0}input[type=range]:focus{outline:0}input[type=range]::-webkit-slider-runnable-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-webkit-slider-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;-webkit-appearance:none;margin-top:-11.5px}input[type=range]:focus::-webkit-slider-runnable-track{background:#EFEFEF}input[type=range]::-moz-range-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-moz-range-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer}input[type=range]::-ms-track{width:100%;height:2px;cursor:pointer;background:0 0;border-color:transparent;color:transparent}input[type=range]::-ms-fill-lower{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-fill-upper{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;height:2px}input[type=range]:focus::-ms-fill-lower{background:#EFEFEF}input[type=range]:focus::-ms-fill-upper{background:#363636}.switch{display:block;position:relative;line-height:22px;font-size:16px;height:22px}.switch input{outline:0;opacity:0;width:0;height:0}.slider{width:50px;height:22px;border-radius:22px;cursor:pointer;background-color:grey}.slider,.slider:before{display:inline-block;transition:.4s}.slider:before{position:relative;content:"";border-radius:50%;height:16px;width:16px;left:4px;top:3px;background-color:#fff}input:checked+.slider{background-color:#ff3034}input:checked+.slider:before{-webkit-transform:translateX(26px);transform:translateX(26px)}select{border:1px solid #363636;font-size:14px;height:22px;outline:0;border-radius:5px}.image-container{position:relative;min-width:160px}.close{position:absolute;right:5px;top:5px;background:#ff3034;width:16px;height:16px;border-radius:100px;color:#fff;text-align:center;line-height:18px;cursor:pointer}.hidden{display:none}
        </style>
        <script src="https:\/\/ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js"></script>
        <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow/tfjs@1.3.1/dist/tf.min.js"></script>
        <!--Add Teachable Machine API-->
        <script src="https:\/\/cdn.jsdelivr.net/npm/@teachablemachine/image@0.8/dist/teachablemachine-image.min.js"></script>  
        <script src="https:\/\/cdn.jsdelivr.net/npm/@teachablemachine/pose@0.8/dist/teachablemachine-pose.min.js"></script>       
    </head>
    <body>
        <section class="main">
            <figure>
              <div id="stream-container" class="image-container hidden">
                <div class="close" id="close-stream" style="display:none">×</div>
                <img id="stream" src="" style="display:none" crossorigin="anonymous">
                <canvas id="canvas" width="0" height="0"></canvas>
              </div>
            </figure>         
            <section id="buttons">
                <table>
                <tr><td><button id="get-still" style="display:none">Get Still</button></td><td><button id="toggle-stream" style="display:none"></td></tr>
                </table>
            </section>        
            <div id="logo">
                <label for="nav-toggle-cb" id="nav-toggle">&#9776;&nbsp;&nbsp;Settings</label>
            </div>
            <div id="content">
                <div id="sidebar">
                    <input type="checkbox" id="nav-toggle-cb">
                    <nav id="menu">
                        <div class="input-group">
                          <label for="kind">TeachableMachine Model Type</label>
                          <select id="kind">
                            <option value="image">image</option>
                            <option value="pose">pose</option>
                          </select>
                        </div>
                        <div class="input-group">
                          <label for="modelPath">Model Path</label>
                          <input type="text" id="modelPath" value="">
                        </div>
                        <div class="input-group">
                            <label for="btnModel"></label>
                            <button type="button" id="btnModel" onclick="LoadModel();">Start Recognition</button>
                        </div>                        
                        <div class="input-group" id="flash-group">
                            <label for="flash">Flash</label>
                            <div class="range-min">0</div>
                            <input type="range" id="flash" min="0" max="255" value="0" step="1" onchange="updateValue(this.id, this.value)">
                            <div class="range-max">255</div>
                        </div>
                        <div class="input-group" id="brightness-group">
                            <label for="brightness">Brightness</label>
                            <div class="range-min">-2</div>
                            <input type="range" id="brightness" min="-2" max="2" step="1" value="0" onchange="updateValue(this.id, this.value)">
                            <div class="range-max">2</div>
                        </div>
                        <div class="input-group" id="contrast-group">
                            <label for="contrast">Contrast</label>
                            <div class="range-min">-2</div>
                            <input type="range" id="contrast" min="-2" max="2" step="1" value="0" onchange="updateValue(this.id, this.value)">
                            <div class="range-max">2</div>
                        </div>
                        <div class="input-group" id="hmirror-group">
                            <label for="hmirror">H-Mirror</label>
                            <div class="range-min">0</div>
                            <input type="range" id="hmirror" min="0" max="1" step="1" value="0" onchange="updateValue(this.id, this.value)">
                            <div class="range-max">1</div>
                        </div>
                        <div class="input-group" id="vflip-group">
                            <label for="vflip">V-Flip</label>
                            <div class="range-min">0</div>
                            <input type="range" id="vflip" min="0" max="1" step="1" value="0" onchange="updateValue(this.id, this.value)">
                            <div class="range-max">1</div>
                        </div>
                    </nav>
                    <div class="input-group">
                            <label for="btnStartRobot"></label>
                            <button type="button" id="btnStartRobot" style="display:none" onclick="StartRobot();">START Robot</button>
                    </div>    
                </div>
            </div>
        </section>
        <br>
        <div id="result" style="color:red"><div>
        
        <script>
          document.addEventListener('DOMContentLoaded', function (event) {
            var baseHost = document.location.origin
            var streamUrl = baseHost + ':81'
            const hide = el => {
              el.classList.add('hidden')
            }
            const show = el => {
              el.classList.remove('hidden')
            }
            const disable = el => {
              el.classList.add('disabled')
              el.disabled = true
            }
            const enable = el => {
              el.classList.remove('disabled')
              el.disabled = false
            }
            const updateValue = (el, value, updateRemote) => {
              updateRemote = updateRemote == null ? true : updateRemote
              let initialValue
              if (el.type === 'checkbox') {
                initialValue = el.checked
                value = !!value
                el.checked = value
              } else {
                initialValue = el.value
                el.value = value
              }
              if (updateRemote && initialValue !== value) {
                updateConfig(el);
              }
            }
            function updateConfig (el) {
              let value
              switch (el.type) {
                case 'checkbox':
                  value = el.checked ? 1 : 0
                  break
                case 'range':
                case 'select-one':
                  value = el.value
                  break
                case 'button':
                case 'submit':
                  value = '1'
                  break
                default:
                  return
              }
              const query = `${baseHost}/control?var=${el.id}&val=${value}`
              console.log(query);
              fetch(query)
                .then(response => {
                  console.log(`request to ${query} finished, status: ${response.status}`)
                })
            }
            document
              .querySelectorAll('.close')
              .forEach(el => {
                el.onclick = () => {
                  hide(el.parentNode)
                }
              }
            )

            const view = document.getElementById('stream')
            const viewContainer = document.getElementById('stream-container')
            const stillButton = document.getElementById('get-still')
            const streamButton = document.getElementById('toggle-stream')
            const closeButton = document.getElementById('close-stream')
            const stopStream = () => {
              //window.stop();
              view.src="";
              streamButton.innerHTML = 'Start Stream'
            }
            const startStream = () => {
              view.src = `${streamUrl}/stream`
              show(viewContainer)
              streamButton.innerHTML = 'Stop Stream'
            }
            // Attach actions to buttons
            stillButton.onclick = () => {
              stopStream()
              try{
                view.src = `${baseHost}/capture?_cb=${Date.now()}`
              }
              catch(e) {
                view.src = `${baseHost}/capture?_cb=${Date.now()}`  
              }
              show(viewContainer)
            }
            closeButton.onclick = () => {
              stopStream()
              hide(viewContainer)
            }
            streamButton.onclick = () => {
              const streamEnabled = streamButton.innerHTML === 'Stop Stream'
              if (streamEnabled) {
                stopStream()
              } else {
                startStream()
              }
            }
            // Attach default on change action
            document
              .querySelectorAll('.default-action')
              .forEach(el => {
                el.onchange = () => updateConfig(el)
              })
          })

          var baseHost = document.location.origin

          function updateValue(element, value) {
            const query = `${baseHost}/control?cmd=${element}&val1=${value}`;
            fetch(query);
          }

          function sendPredictions(class1, prob1, class2, prob2, class3, prob3) {
            const query = `${baseHost}/control?cmd=TeachableMachineResults&var1=${class1}&val1=${prob1}&var2=${class2}&val2=${prob2}&var3=${class3}&val3=${prob3}`;
            fetch(query);
          }

        </script>
          
        <script>
        var getStill = document.getElementById('get-still');
        var ShowImage = document.getElementById('stream');
        var canvas = document.getElementById("canvas");
        var context = canvas.getContext("2d");  
        var modelPath = document.getElementById('modelPath');
        var result = document.getElementById('result');
        var kind = document.getElementById('kind');        
        let Model;
        
        async function LoadModel() {
          if (modelPath.value=="") {
            result.innerHTML = "Please input model path.";
            return;
          }
    
          result.innerHTML = "Please wait for loading model.";
          
          const URL = modelPath.value;
          const modelURL = URL + "model.json";
          const metadataURL = URL + "metadata.json";
          if (kind.value=="image") {
            Model = await tmImage.load(modelURL, metadataURL);
          }
          else if (kind.value=="pose") {
            Model = await tmPose.load(modelURL, metadataURL);
          }
          maxPredictions = Model.getTotalClasses();
          result.innerHTML = "";
    
          getStill.click();
          document.getElementById('btnStartRobot').style.display = "inline";
        }
    
        async function predict() {
          var data = "";
    
          canvas.setAttribute("width", ShowImage.width);
          canvas.setAttribute("height", ShowImage.height);
          context.drawImage(ShowImage, 0, 0, ShowImage.width, ShowImage.height); 
           
          if (kind.value=="image")
            var prediction = await Model.predict(canvas);
          else if (kind.value=="pose") {
            var { pose, posenetOutput } = await Model.estimatePose(canvas);
            var prediction = await Model.predict(posenetOutput);
          }

          //Reorder to get classes in descending order on probability    
          if (maxPredictions>0) {
            for (let i = 0; i < maxPredictions; i++) {
              if (i > 0 && prediction[i].probability > prediction[i-1].probability) {
                var temp_class = prediction[i-1].className;
                var temp_probability = prediction[i-1].probability;
                prediction[i-1].className = prediction[i].className;
                prediction[i-1].probability = prediction[i].probability;
                prediction[i].className = temp_class;
                prediction[i].probability = temp_probability;
              }              
            }

            for (let i = 0; i < maxPredictions; i++)
              data += prediction[i].className + "," + prediction[i].probability.toFixed(2) + "<br>";
            
            //Sending TeachableMachine Results via HTTP Query to Main Program
            sendPredictions(prediction[0].className, prediction[0].probability, prediction[1].className, prediction[1].probability, prediction[2].className, prediction[2].probability);
            
            result.innerHTML = data;
            result.innerHTML += "<br>Result: " + prediction[0].className + "<br>Result Probability: " + prediction[0].probability;
          }
          else
            result.innerHTML = "Unrecognizable";
            
          getStill.click();
        }

        async function StartRobot(){
          if(document.getElementById('btnStartRobot').innerHTML == "START Robot"){
            updateValue("StartRobot", 1);
            document.getElementById('btnStartRobot').innerHTML = "STOP Robot";
          }
          else
          {
            updateValue("StartRobot", 0);
            document.getElementById('btnStartRobot').innerHTML = "START Robot";
          }            
        }
    
        ShowImage.onload = function (event) {
          if (Model) {
          try { 
            document.createEvent("TouchEvent");
            setTimeout(function(){predict();},250);
          }
          catch(e) { 
            predict();
          } 
          }
        }    
        </script>
    </body>
</html>)rawliteral";

static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
      .uri       = "/",
      .method    = HTTP_GET,
      .handler   = index_handler,
      .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
      .uri       = "/control",
      .method    = HTTP_GET,
      .handler   = cmd_handler,
      .user_ctx  = NULL
  }; 
  
  //http://[...IP...]:81/stream
  httpd_uri_t stream_uri = {
      .uri       = "/stream",
      .method    = HTTP_GET,
      .handler   = stream_handler,
      .user_ctx  = NULL
  };

    httpd_uri_t capture_uri = {
      .uri       = "/capture",
      .method    = HTTP_GET,
      .handler   = capture_handler,
      .user_ctx  = NULL
  };
  
  Serial.printf("Starting web server on port: '%d'\n", config.server_port);  //Server Port
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
      httpd_register_uri_handler(camera_httpd, &index_uri);
      httpd_register_uri_handler(camera_httpd, &cmd_uri);
      httpd_register_uri_handler(camera_httpd, &capture_uri);
  }
  
  config.server_port += 1;  //Stream Port
  config.ctrl_port += 1;    //UDP Port
  Serial.printf("Starting stream server on port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
      httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.print(WiFi.localIP());
  
  // Start streaming web server
  startCameraServer();
}

void loop() {
  delay(1);
}