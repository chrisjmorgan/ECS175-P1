//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#include "util.hpp"
#include "comm.hpp"
#include "../../imgui/globals.hpp"
// force imgui to use GLAD
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
//#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

std::vector<Polygon> polygons;
int pid = 0; // current polygon
static bool show_gui = false;

const int LEFT = 1;
const int RIGHT = 2;
const int BOTTOM = 4;
const int TOP = 8;
const int INSIDE = 0;

//clipping perams - debbugging only
const int x_max = 10; 
const int y_max = 8; 
const int x_min = 4; 
const int y_min = 4; 


//computes the region code for clipping
int computeRegion(double x, double y) {

  int region = 0;
  
  if (x < x_min)  
    region |= LEFT; 
  else if (x > x_max)  
    region |= RIGHT; 
  if (y < y_min) 
    region |= BOTTOM; 
  else if (y > y_max)  
    region |= TOP; 
  
  return region;
}

//cohen-Sutherland Clip
void csClipping(double x1, double y1, double x2, double y2) {

  int region1 = computeRegion(x1, y1);
  int region2 = computeRegion(x2, y2);

  bool accept = false;

  while (true) {
    if ((region1 == 0) && (region2 == 0)) {

      accept = true;
      break;
    } else if (region1 & region2) {

      break;
    } else {

      int code_out;
      double x, y;

      if (region1 != 0)
        code_out = region1;
      else
        code_out = region2;

      if (code_out & TOP) {

        x = x1 + (x2 - x1) * (y_max - y1) / (y2 - y1);
        y = y_max;
      } else if (code_out & BOTTOM) {

        x = x1 + (x2 - x1) * (y_min - y1) / (y2 - y1);
        y = y_min;
      } else if (code_out & RIGHT) {

        y = y1 + (y2 - y1) * (x_max - x1) / (x2 - x1);
        x = x_max;
      } else if (code_out & LEFT) {

        y = y1 + (y2 - y1) * (x_min - x1) / (x2 - x1);
        x = x_min;
      }

      if (code_out == region1) {
        x1 = x;
        y1 = y;
        region1 = computeRegion(x1, y1);
      } else {
        x2 = x;
        y2 = y;
        region2 = computeRegion(x2, y2);
      }
    }
  }
}


static bool
CapturedByGUI()
{
    ImGuiIO& io = ImGui::GetIO();
    return (io.WantCaptureMouse);
}


void
KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  //---------------------------------------------------
  // TODO finish the implementation starting from here
  //---------------------------------------------------
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    // close window
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  else if (key == GLFW_KEY_G && action == GLFW_PRESS) {
    show_gui = !show_gui;
  }

}

void
CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
  //---------------------------------------------------
  // TODO finish the implementation starting from here
  //---------------------------------------------------
  if (!CapturedByGUI()) {
    int left_state  = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    int right_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    // left click
    if (left_state == GLFW_PRESS) {
    }
    else {
    }
    
    // right click
    if (right_state == GLFW_PRESS) {
    }
    else {
    }
  }
 
}

void DDA(GLFWwindow* window, int x1, int y1, int x2, int y2){
  int dx = x2-x1;
  int dy = y2-y1;

  int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

  float Xinc = dx / (float) steps;
  float Yinc = dy / (float) steps;

  float x = x1;
  float y = y1;

  for (int i = 0; i <= steps; i++)
  {
    MakePix(window, x, y, 0, 1.0, 0);
    x += Xinc;
    y += Yinc;
  }
}

void bresenham(GLFWwindow* window, int x1, int y1, int x2, int y2)
{
  int m = 2 * (y2-y1);
  int sen = m - (x2-x1);

  for (int x = x1, y = y1; x <= x2; x++) 
   { 
    
      MakePix(window, x, y, 0, 1.0, 0); 
      // Add slope to increment angle formed 
      sen += m; 
  
      // Slope error reached limit, time to 
      // increment y and update slope error. 
      if (sen >= 0) 
      { 
         y++; 
         sen  -= 2 * (x2 - x1); 
      } 
   }

}


void drawPolysDDA(GLFWwindow* window){


  for (int i = 0; i < polygons.size(); i++){
    for (int j = 0; j < polygons[i].points.size(); j++)
    {

  DDA(window, polygons[i].points[j].x, polygons[i].points[j].y ,polygons[i].points[(j+1)%polygons[i].points.size()].x, polygons[i].points[(j+1)%polygons[i].points.size()].y);

    }
  }
}

void drawPolysBresen(GLFWwindow* window)
{

  for (int i = 0; i < polygons.size(); i++){
    for (int j = 0; j < polygons[i].points.size(); j++)
    {

  bresenham(window, polygons[i].points[j].x, polygons[i].points[j].y ,polygons[i].points[(j+1)%polygons[i].points.size()].x, polygons[i].points[(j+1)%polygons[i].points.size()].y);

    }
  }

// bresenham(window, 5, 5, 500, 5);

 }


void
DrawCall(GLFWwindow* window, bool drawFlag1)
{

    if (!drawFlag1)
    drawPolysDDA(window);
    else if (drawFlag1)
      drawPolysBresen(window);

}




int
main(const int argc, const char** argv)
{

  int width = 1000, height = 800;

  if (argc < 2) 
    throw std::runtime_error("missing input file"); 
  
  ReadFile(argv[1]);
  // Compute the Center of Mass
  for (auto& p : polygons) {        
    float cx = 0.f, cy = 0.f;
    for (auto& pt : p.points) {
      cx += pt.x;
      cy += pt.y;
    }
    cx /= (float)p.points.size();
    cy /= (float)p.points.size();
    p.cx = cx;
    p.cy = cy;
  }

  //---------------------------------------------------
  // TODO finish the implementation starting from here
  //---------------------------------------------------

  glfwSetErrorCallback(ErrorCallback);
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  // Provide Window Hint
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
  

  // OpenGL Setup
  GLFWwindow* window = NULL;
  window = glfwCreateWindow(width, height, "ECS 175 Renderer", NULL, NULL);
  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  // Ready
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Load GLAD symbols
  int err = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0;
  if (err) {
    throw std::runtime_error("Failed to initialize OpenGL loader!");
  }

  // Callback
  glfwSetKeyCallback(window, KeyCallback);
  //glfwSetWindowSizeCallback(window, WindowSizeCallback);
  glfwSetCursorPosCallback(window, CursorPositionCallback);
  

 // ImGui
  {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark(); // or ImGui::StyleColorsClassic();
    
    // Initialize Dear ImGui
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();
  }

  // Execute
  while (!glfwWindowShouldClose(window)) {
   glClear(GL_COLOR_BUFFER_BIT);
    
    //std::cout << globe.drawFlag << std::endl;

    if (globe.drawFlag)
    DrawCall(window,globe.drawAlg);

    {
      // Initialization
      ImGui_ImplOpenGL2_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      
      // - Uncomment below to show ImGui demo window
      if (show_gui)
	ImGui::ShowDemoWindow(&show_gui);
      
      // Render GUI
      ImGui::Render();
      ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Exit
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  // Exit
  glfwDestroyWindow(window);
  glfwTerminate();










  return EXIT_SUCCESS;
}
