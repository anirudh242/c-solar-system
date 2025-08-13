#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define PI 3.1415926535897932384626433832
#define G 6.67430e-3f // Scaled gravitational constant

// Scale: 1 AU (~1.5e8 km) = 200 units in OpenGL coords
#define SCALE_DISTANCE 200.0f
#define TIME_MULTIPLIER 100.0f // Time speed-up factor

typedef struct
{
    float x, y;
    float vx, vy;
    float mass;
    float radius;
    float r, g, b;
} Body;

typedef struct
{
    float x, y;
} Point;

typedef struct
{
    Body body;
    Point *trail;
    int trailCount;
    int showTrail;
} Planet;

#define PLANET_COUNT 8

// Calculate circular orbital velocity for a planet at distance r from sun
float orbital_velocity(float M, float r)
{
    return sqrtf(G * M / r);
}

void add_trail_point(Planet *p, float x, float y)
{
    p->trail = realloc(p->trail, (p->trailCount + 1) * sizeof(Point));
    p->trail[p->trailCount].x = x;
    p->trail[p->trailCount].y = y;
    p->trailCount++;
}

void render_trail(Planet *p)
{
    if (!p->showTrail || p->trailCount < 2)
        return;

    glColor3f(p->body.r, p->body.g, p->body.b);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < p->trailCount; i++)
    {
        glVertex2f(p->trail[i].x, p->trail[i].y);
    }
    glEnd();
}

void render_body(Body b)
{
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(b.r, b.g, b.b);
    glVertex2f(b.x, b.y);
    for (int i = 0; i <= 50; i++)
    {
        float angle = i * 2.0f * PI / 50;
        glVertex2f(b.x + cos(angle) * b.radius,
                   b.y + sin(angle) * b.radius);
    }
    glEnd();
}

void update_physics(Body *planet, Body sun, float dt)
{
    float dx = sun.x - planet->x;
    float dy = sun.y - planet->y;
    float distSq = dx * dx + dy * dy;

    if (distSq < 1.0f)
        return;

    float dist = sqrtf(distSq);
    float accel = (G * sun.mass) / distSq;
    float ax = accel * (dx / dist);
    float ay = accel * (dy / dist);

    planet->vx += ax * dt;
    planet->vy += ay * dt;

    planet->x += planet->vx * dt;
    planet->y += planet->vy * dt;
}

void render_scene(Body sun, Planet planets[], int count)
{
    glClear(GL_COLOR_BUFFER_BIT);
    render_body(sun);
    for (int i = 0; i < count; i++)
    {
        render_body(planets[i].body);
        render_trail(&planets[i]);
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    static int trailsOn = 1;
    if (action == GLFW_PRESS && key == GLFW_KEY_T)
    {
        trailsOn = !trailsOn;
        Planet *planets = glfwGetWindowUserPointer(window);
        if (planets)
        {
            for (int i = 0; i < PLANET_COUNT; i++)
                planets[i].showTrail = trailsOn;
        }
    }
}

// FOr resizing the window
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);

    // Keep aspect ratio
    float aspect = (float)width / (float)height;
    float viewSize = 1000.0f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (aspect >= 1.0f)
    {
        glOrtho(-viewSize * aspect, viewSize * aspect, -viewSize, viewSize, -1.0, 1.0);
    }
    else
    {
        glOrtho(-viewSize, viewSize, -viewSize / aspect, viewSize / aspect, -1.0, 1.0);
    }
    glMatrixMode(GL_MODELVIEW);
}

int main()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to init GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow *window = glfwCreateWindow(1600, 900, "Solar System Simulation", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to init GLAD\n");
        return -1;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-400, 400, -300, 300, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    Body sun = {0, 0, 0, 0, 1.989e6f, 30, 1.0f, 1.0f, 0.0f};

    Planet planets[PLANET_COUNT] = {
        // Mercury
        {{0.39f * SCALE_DISTANCE, 0, 0, 0, 0.330f * 1e3f, 5, 0.5f, 0.5f, 0.5f}, NULL, 0, 1},
        // Venus
        {{0.72f * SCALE_DISTANCE, 0, 0, 0, 4.87f * 1e3f, 7, 1.0f, 0.7f, 0.0f}, NULL, 0, 1},
        // Earth
        {{1.0f * SCALE_DISTANCE, 0, 0, 0, 5.972f * 1e3f, 10, 0.0f, 0.0f, 1.0f}, NULL, 0, 1},
        // Mars
        {{1.52f * SCALE_DISTANCE, 0, 0, 0, 0.642f * 1e3f, 8, 1.0f, 0.0f, 0.0f}, NULL, 0, 1},
        // Jupiter
        {{5.20f * SCALE_DISTANCE, 0, 0, 0, 1898.0f * 1e3f, 20, 1.0f, 0.5f, 0.0f}, NULL, 0, 1},
        // Saturn
        {{9.58f * SCALE_DISTANCE, 0, 0, 0, 568.0f * 1e3f, 17, 1.0f, 1.0f, 0.5f}, NULL, 0, 1},
        // Uranus
        {{19.20f * SCALE_DISTANCE, 0, 0, 0, 86.8f * 1e3f, 15, 0.5f, 1.0f, 1.0f}, NULL, 0, 1},
        // Neptune
        {{30.05f * SCALE_DISTANCE, 0, 0, 0, 102.0f * 1e3f, 15, 0.0f, 0.0f, 0.5f}, NULL, 0, 1}};

    // Set initial velocities for circular orbits (tangential direction)
    for (int i = 0; i < PLANET_COUNT; i++)
    {
        float r = sqrtf(planets[i].body.x * planets[i].body.x +
                        planets[i].body.y * planets[i].body.y); // actual distance between the bodies
        float v = orbital_velocity(sun.mass, r);
        planets[i].body.vx = 0;
        planets[i].body.vy = v;
        printf("%d: r=%.2f v=%.4f\n", i, r, v);
    }

    glfwSetWindowUserPointer(window, planets);

    double prevTime = glfwGetTime();
    const float fixedDt = 0.001f;
    float accumulator = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        float frameTime = (float)(currentTime - prevTime);
        prevTime = currentTime;

        if (frameTime > 0.1f)
            frameTime = 0.1f;

        accumulator += frameTime;

        while (accumulator >= fixedDt)
        {
            for (int i = 0; i < PLANET_COUNT; i++)
            {
                update_physics(&planets[i].body, sun, fixedDt * TIME_MULTIPLIER);
                add_trail_point(&planets[i], planets[i].body.x, planets[i].body.y);
            }
            accumulator -= fixedDt;
        }

        render_scene(sun, planets, PLANET_COUNT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (int i = 0; i < PLANET_COUNT; i++)
        free(planets[i].trail);

    glfwTerminate();
    return 0;
}
