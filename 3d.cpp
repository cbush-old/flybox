#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <vector>
#include <fstream>
#include <cstdint>

#include <SDL2/SDL.h>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GL/gl.h>
#include <GL/glext.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>


const int blocksqroot = 40;


struct SDL_GL_app {
  int init[3] { 
    SDL_Init(SDL_INIT_EVERYTHING),
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3),
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0)
  };
  static const int w { 1366 }, h { 768 };
  SDL_Window *window {
    SDL_CreateWindow(
      "",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      //1366, 768,
      w, h,
      SDL_WINDOW_OPENGL
    )
  };
  SDL_GLContext context {
    SDL_GL_CreateContext(window)
  };
  struct GL_app {
    GLuint
      program { glCreateProgram() },
      shaderv { glCreateShader(GL_VERTEX_SHADER) },
      shaderf { glCreateShader(GL_FRAGMENT_SHADER) };
    
    inline void ready_shader(GLuint id, const char * path){
      std::ifstream f (path);
      GLint length = f.seekg(0, f.end).tellg();
      char *src = new char[length];
      f.seekg(0).read(src, length);
      glShaderSource(id, 1, &src, &length);
      shader_log(id);
      delete[] src;
      glCompileShader(id);
      shader_log(id);
      glAttachShader(program, id);
      shader_log(id);
      program_log();
    }
    
    void shader_log(GLuint id) const {
      int log_length, max_length;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &max_length);
      char* log = new char[max_length];
      glGetShaderInfoLog(id, max_length, &log_length, log);
      if(log_length > 0) 
        std::cout << log << '\n';
      delete[] log;
    }
    
    void program_log() const {
      int log_length, max_length;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);
      char* log = new char[max_length];
      glGetProgramInfoLog(program, max_length, &log_length, log);
      if(log_length > 0) 
        std::cout << log << '\n';
      delete[] log;
    }

    GL_app(){
      glViewport(0, 0, w, h);
      
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      
      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CW);
      
      glEnable(GL_DEPTH_TEST);
      
      ready_shader(shaderv, "shader.glvs");
      ready_shader(shaderf, "shader.glfs");
      glLinkProgram(program);
      program_log();
      glUseProgram(program);
      program_log();
    }
    
    ~GL_app(){
      glDeleteProgram(program);
      glDeleteShader(shaderv);
      glDeleteShader(shaderf);
    }
    
  } gl;
  
  GLint get_attrib(const char * name){
    return glGetAttribLocation(gl.program, name);
  }

  GLint get_uniform(const char * name){
    return glGetUniformLocation(gl.program, name);
  }
  
  SDL_GL_app(){
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, init);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, init + 1);
    std::cout << "OpenGL " << init[0] << '.' << init[1] << '\n';
  
  }
  
  ~SDL_GL_app(){
    std::cout << SDL_GetError() << '\n';
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  inline void swap(){
    SDL_GL_SwapWindow(window);
  }
  
} App;

template <typename T>
struct VBO {
  GLuint id;
  inline void bind(){
    glBindBuffer(GL_ARRAY_BUFFER, id);
  }
  inline void unbind(){
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  VBO(std::initializer_list<T> v){
    glGenBuffers(1, &id);
    bind();
    glBufferData(
      GL_ARRAY_BUFFER,
      (GLsizeiptr)(v.size() * sizeof(T)),
      (void*)(v.begin()),
      GL_STATIC_DRAW
    );
    unbind();
  }
  ~VBO(){
    glDeleteBuffers(1, &id);
  }
};


int main(int argc, const char * argv[]){
 
  GLint 
    vpos = App.get_attrib("position"),
    color = App.get_uniform("color"),
    texcoord = App.get_attrib("texcoord_in"),
    modelview = App.get_uniform("modelview"),
    normal_mat = App.get_uniform("normal_matrix"),
    projection = App.get_uniform("projection"),
    a_color = App.get_uniform("ambient_color"),
    a_intensity = App.get_uniform("ambient_intensity"),
    a_direction = App.get_uniform("ambient_direction"),
    normal_in = App.get_attrib("normal_in");
  
  std::cout << "Texcoord: " << a_color << '\n';
  
  glUniform4f(color, 1.0, 1.0, 1.0, 1.0);
  glUniform3f(a_color, 1.0, 0.9, 0.8);
  glUniform1f(a_intensity, 0.1);
  glUniform3f(a_direction, 0.4, -0.5, 0.0);
  
  GLfloat x { 10.f }, y { 10.f }, z { 10.0f };
  
  glm::mat4 projection_matrix {
    glm::perspective<GLfloat>(
      45,
      float(SDL_GL_app::w) / float(SDL_GL_app::h),
      10.0,
      5000.0
    )
  };
  
  glUniformMatrix4fv(
    projection,
    1,
    GL_FALSE,
    glm::value_ptr(projection_matrix)
  );
  
  GLfloat t { 1.0f };
  
  
  #define V1 -x, +y, +z
  #define V2 +x, +y, +z
  #define V3 -x, -y, +z
  #define V4 +x, -y, +z
  #define V5 -x, +y, -z
  #define V6 +x, +y, -z
  #define V7 -x, -y, -z
  #define V8 +x, -y, -z

  VBO<GLfloat> vbo {

    V1, V2, V3, V4,
    V6, V5, V8, V7,
    V5, V1, V7, V3,
    V2, V6, V4, V8,
    V5, V6, V1, V2,
    V3, V4, V7, V8,

    t, 0, t, t, 0, 0, 0, t,
    t, 0, t, t, 0, 0, 0, t,
    t, 0, t, t, 0, 0, 0, t,
    t, 0, t, t, 0, 0, 0, t,
    t, 0, t, t, 0, 0, 0, t,
    t, 0, t, t, 0, 0, 0, t,
    
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,

  };
  #undef V1
  #undef V2
  #undef V3
  #undef V4
  #undef V5
  #undef V6
  #undef V7
  #undef V8

  vbo.bind();
  glVertexAttribPointer(vpos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glVertexAttribPointer(
    texcoord, 2, GL_FLOAT, GL_FALSE, 0, (void*)(24 * 3 * sizeof(GLfloat))
  );
  glVertexAttribPointer(
    normal_in, 3, GL_FLOAT, GL_FALSE, 0, (void*)(24 * 5 * sizeof(GLfloat))
  );
  vbo.unbind();
  
  GLuint texture;
  
  glGenTextures(1, &texture);

  std::vector<uint16_t> pixels (0x10000);
  
  for(int i = 0; i < 0x10000; ++i){
    //pixels[i] = int(0x8000 + sin(double(i)*0.01)*0x8000)|0x000f;
    pixels[i] = (bool((i/4)%4) * (((i/256)%32)>>2) * 0xfff0)|0x000f;
  }
    
  glBindTexture(GL_TEXTURE_2D, texture);
    
  glTexImage2D(
    GL_TEXTURE_2D, 
    0, 
    GL_RGBA, 
    256,
    256,
    0,
    GL_RGBA,
    GL_UNSIGNED_SHORT_4_4_4_4, 
    (const GLvoid*)pixels.data()
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  glClearColor(0.5, 0.5, 0.6, 1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  glEnableVertexAttribArray(vpos);
  glEnableVertexAttribArray(texcoord);
  glEnableVertexAttribArray(normal_in);
  
  glBindTexture(GL_TEXTURE_2D, texture);

  
  float 
    movx { 0.0f },
    movy { 0.0f },
    movz { 0.0f },
    lookx { 0.0f },
    looky { 0.0f },
    lookz { 0.0f };
    
  glm::vec4 pos (0.0f, 0.0f, -70.0f, 0.0f);
  glm::vec3 look (0.0f, 0.0f, 0.0f);

  glm::quat rotation;
  
  float rx { 0.0f }, ry { 0.0f }, rz { 0.0f };
  int addblock;
  
  float framerate = 0.0f;
  
  std::vector<float> blockh (blocksqroot * blocksqroot);
  float prev = -20.0f;
  for(int i = 0; i < blockh.size(); ++i){
    //i = float(rand()%10)*20.0f - 20.0f;
    //blockh[i] = (prev += (rand()&1)*2 - 1) * 20.0f;
    //prev *= bool(i%blocksqroot);
    blockh[i] = rand()%3000;
  }
  
  glm::vec4 vel;
  
  for(bool done = false; !done;){
    
    clock_t tock = clock();
    
    for(SDL_Event e; SDL_PollEvent(&e);){
      if(e.type==SDL_QUIT){
        done = true;
      
      } else if(e.type==SDL_KEYDOWN){
        switch(e.key.keysym.sym){
          case SDLK_DOWN: looky = -1; break;
          case SDLK_UP: looky = +1; break;
          case SDLK_LEFT: lookx = -1; break;
          case SDLK_RIGHT: lookx = +1; break;
          case SDLK_w: movz = +1; break;
          case SDLK_s: movz = -1; break;
          case SDLK_a: movx = -1; break;
          case SDLK_d: movx = +1; break;
          case SDLK_q: movy = +1; break;
          case SDLK_e: movy = -1; break;
          case SDLK_c: lookz = -1; break;
          case SDLK_z: lookz = +1; break;
          case SDLK_r: addblock = 1; break;
          case SDLK_t: addblock = -1; break;
          case SDLK_p: done = true; break;
        }
      } else if(e.type==SDL_KEYUP){
        switch(e.key.keysym.sym){
          case SDLK_DOWN: if(looky < 0) looky = 0; break;
          case SDLK_UP: if(looky > 0) looky = 0; break;
          case SDLK_LEFT: if(lookx < 0) lookx = 0; break;
          case SDLK_RIGHT: if(lookx > 0) lookx = 0; break;
          case SDLK_w: if(movz > 0) movz = 0; break;
          case SDLK_s: if(movz < 0) movz = 0; break;
          case SDLK_a: if(movx < 0) movx = 0; break;
          case SDLK_d: if(movx > 0) movx = 0; break;
          case SDLK_q: if(movy > 0) movy = 0; break;
          case SDLK_e: if(movy < 0) movy = 0; break;
          case SDLK_c: if(lookz < 0) lookz = 0; break;
          case SDLK_r: case SDLK_t: addblock = 0; break;
          case SDLK_z: if(lookz > 0) lookz = 0; break;
        }
      }
    }


    
    if(lookx){
      // left-right is always on the world Y axis
      glm::quat rot = glm::angleAxis(lookx, glm::vec3(0.0f, 1.0f, 0.0f));
      rotation = glm::normalize(rotation * rot);
    }
    
    if(looky){
      glm::vec3 axisx (1.0f, 0.0f, 0.0f);
      
      axisx = glm::normalize(axisx * rotation);
      glm::quat rot = glm::angleAxis(-looky, axisx);
      rotation = glm::normalize(rotation * rot);
      
    }

    
    
    glm::vec4 movement (-movx, movy, movz, 0.0f);
    
    movement *= 0.1f;
    vel += movement;
    movement += vel;
    
    
    
    movement = movement * rotation;
    
    glm::vec3 r (0.0f, 0.0f, 0.0f);
    
    glm::mat4 translation {
      glm::translate(
        glm::mat4_cast(rotation),
        glm::vec3(pos)
      )
    };

    pos += movement;
    
    
    for(float j = 0; j < (float)blocksqroot; ++j)
    for(float i = 0; i < (float)blocksqroot; ++i){
      
      glm::mat4 blockpos {
        glm::translate(glm::mat4(), glm::vec3(i * 20.0f, blockh[int(j * blocksqroot + i)], j * 20.0f))
      };
      
      glUniformMatrix4fv(
        modelview,
        1,
        GL_FALSE,
        glm::value_ptr(translation * blockpos)
      );
      
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 24);

    }
    
    App.swap();  
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    //SDL_Delay(10);
    clock_t tick = clock();
    framerate = (framerate + (tick - tock))/2.0f;
    
  }

  std::cout << (blockh.size()) << " blocks\n";
  std::cout << (1.0f/(framerate/CLOCKS_PER_SEC)) << '\n';
  
  glDeleteTextures(1, &texture);
  
}
