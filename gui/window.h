#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>  // Include GLEW header before OpenGL headers
#include <GL/gl.h>
#include <nanogui/nanogui.h>
#include <nanogui/glutil.h>
#include <iostream>
#include <fstream>
#include "Lattice.h"
#include "LatticeSetup.h"
#include "MoveManager.h"
#include "ConfigurationSpace.h"
#include "Scenario.h"
#include "Isometry.h"

class MainWindow : public nanogui::Screen {
public:
    MainWindow() : nanogui::Screen(Eigen::Vector2i(800, 600), "NanoGUI Window") {
        using namespace nanogui;

        // Initialize GLEW
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return;
        }

        Window *window = new Window(this, "Control Panel");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());

        Button *runButton = new Button(window, "Run");
        runButton->setCallback([this]() {
            // Your callback code here
        });
    }
};

#endif // WINDOW_H