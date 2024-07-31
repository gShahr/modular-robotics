#include "window.h"

MainWindow::MainWindow() : nanogui::Screen(Eigen::Vector2i(800, 600), "NanoGUI Window") {
    using namespace nanogui;

    Window *window = new Window(this, "Control Panel");
    window->setPosition(Vector2i(15, 15));
    window->setLayout(new GroupLayout());

    Button *runButton = new Button(window, "Run");
    runButton->setCallback([this]() {
        this->runTasks();
    });

    performLayout();
}

void MainWindow::runTasks() {
    Lattice::setFlags(false);
    LatticeSetup::setupFromJson("input.json");
    std::cout << Lattice::ToString();
    MoveManager::InitMoveManager(Lattice::Order(), Lattice::AxisSize());
    MoveManager::RegisterAllMoves();
    std::cout << "BFS Testing:\n";
    Configuration start(Lattice::GetModuleInfo());
    Configuration end = LatticeSetup::setupFinalFromJson("docs/examples/basic_3d_final.json");
}

MainWindow::~MainWindow() {}

int main(int argc, char **argv) {
    try {
        nanogui::init();

        {
            nanogui::ref<MainWindow> app = new MainWindow();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::cerr << "Caught a fatal error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}