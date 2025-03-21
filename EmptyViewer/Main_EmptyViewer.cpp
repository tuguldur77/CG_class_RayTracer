#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

// Window dimensions
int Width = 512; 
int Height = 512;
vector<vec3> OutputImage(Width* Height, vec3(0.0f)); 
 
// Camera parameters 
vec3 eye(0.0f, 0.0f, 0.0f); 
vec3 u(1.0f, 0.0f, 0.0f), v(0.0f, 1.0f, 0.0f), w(0.0f, 0.0f, 1.0f); 
float l = -0.1f, r = 0.1f, b = -0.1f, t = 0.1f, d = 0.1f; 

// Ray structure
struct Ray {
    vec3 origin, direction;
    Ray(const vec3& o, const vec3& d) : origin(o), direction(normalize(d)) {}
};

// Sphere structure
struct Sphere {
    vec3 center;
    float radius;
    vec3 color;

    bool intersect(const Ray& ray, float& t) const {
        vec3 oc = ray.origin - center;
        float a = dot(ray.direction, ray.direction);
        float b = 2.0f * dot(oc, ray.direction);
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant < 0) return false;
        using std::sqrt;
        float sqrt_d = sqrt(discriminant);
        float t1 = (-b - sqrt_d) / (2.0f * a);
        float t2 = (-b + sqrt_d) / (2.0f * a);

        t = (t1 > 0) ? t1 : t2;
        return t > 0;
    }
};

// Plane structure
struct Plane {
    vec3 normal;
    float d;
    vec3 color;

    bool intersect(const Ray& ray, float& t) const {
        float denom = dot(normal, ray.direction);
        if (abs(denom) > 1e-6) {
            t = -(dot(normal, ray.origin) + d) / denom;
            return (t > 0);
        }
        return false;
    }
};

// Render function
void render() {

    // Scene objects
    vector<Sphere> spheres = {
        { vec3(-4.0f, 0.0f, -7.0f), 1.0f},
        { vec3(0.0f, 0.0f, -7.0f), 2.0f},
        { vec3(4.0f, 0.0f, -7.0f), 1.0f}
    };
    
    // White plane at y = -2 
    Plane plane = { vec3(0.0f, 1.0f, 0.0f), 2.0f, vec3(1.0f) }; 

    // Ray trace for each pixel
    for (int iy = 0; iy < Height; ++iy) {
        for (int ix = 0; ix < Width; ++ix) {
            // Generate ray direction based on pixel coordinates
            float u_ = l + (r - l) * (ix + 0.5f) / Width;
            float v_ = b + (t - b) * (iy + 0.5f) / Height;
            vec3 M = eye + u_ * u + v_ * v - d * w;
            Ray ray(eye, M - eye);

            vec3 color(0.0f); // Default color: black (no intersection)
            float min_t = numeric_limits<float>::max(); // Initialize with maximum value

            // Check for intersections with spheres
            for (const auto& sphere : spheres) {
                float t;
                if (sphere.intersect(ray, t) && t < min_t) {
                    min_t = t; // Update closest intersection distance
                    color = vec3(1.0f, 1.0f, 1.0f); // Set pixel color to white (intersection found)
                }
            }

            // Check for intersection with the plane
            float t_plane;
            if (plane.intersect(ray, t_plane) && t_plane < min_t) {
                min_t = t_plane; // Update closest intersection distance
                color = vec3(1.0f, 1.0f, 1.0f); // Set pixel color to white (intersection found)
            }

            // Store the resulting color for this pixel
            OutputImage[iy * Width + ix] = color;
        }
    }
}


void resize_callback(GLFWwindow*, int nw, int nh) {
    Width = nw;
    Height = nh;
    glViewport(0, 0, nw, nh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(Width), 0.0, static_cast<double>(Height), 1.0, -1.0);
    OutputImage.reserve(Width * Height * 3);
    render();
}

int main(int argc, char* argv[]) {
    GLFWwindow* window;
    if (!glfwInit()) return -1;
    window = glfwCreateWindow(Width, Height, "OpenGL Viewer(Ray Trace - 202213827 투굴두루)", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resize_callback);
    resize_callback(NULL, Width, Height);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, &OutputImage[0]);
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

