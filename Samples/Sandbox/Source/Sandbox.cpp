#include <Reality.h>

#include "TestGameobject.h"

using namespace Reality;
int main() {
    /*
    // Log class and macros.
    // Configure logging
    Log::GetInstance().SetLevel(Reality::LogLevel::Debug);
    Log::GetInstance().SetLogFile("engine.log");
    Log::GetInstance().EnableFileOutput(false);
    Log::GetInstance().EnableColors(true);

    // Log messages at different levels
    RLOG_TRACE("This is a trace message");
    RLOG_DEBUG("Debug class");
    RLOG_INFO("Application started successfully");
    RLOG_WARNING("This is a warning");
    RLOG_ERROR("An error occurred, error code: %d",301);
    RLOG_FATAL("Critical failure!");

    // Using the class directly
    Log::GetInstance().Info("Direct access to logger");

    // Config class.
    // Set config values.
    RLOG_DEBUG("Config class");
    Config::GetInstance().Set("Display","Width","1920");
    Config::GetInstance().Set("Display","Height","1080");
    Config::GetInstance().Set("Display", "VSync", "true");
    Config::GetInstance().Set("Display", "Gamma", "0.5");
    Config::GetInstance().Set("Audio", "Volume", "0.8");
    Config::GetInstance().Set("Audio", "MusicEnabled", "true");

    // Get config values.
    const std::string widthStr = Config::GetInstance().Get("Display", "Width");
    const int width = Config::GetInstance().GetInt("Display", "Width");
    const bool vsync = Config::GetInstance().GetBool("Display", "VSync");
    const float gamma = Config::GetInstance().GetFloat("Display", "Gamma");
    RLOG_INFO("Config Display width str: %s",widthStr.c_str());
    RLOG_INFO("Config Display width: %d",width);
    RLOG_INFO("Config Display gamma: %f",gamma);
    RLOG_INFO("Config Display vsync: %s",vsync ? "true" : "false");
    RLOG_INFO("Config Display gamma: %f",gamma);

    // Save config ini file.
    if (Config::GetInstance().Save("d:/AppConfig.ini")) {
        RLOG_INFO("Config saved successfully");
    } else {
        RLOG_ERROR("Failed to save configuration");
    }

    // Clear current configuration
    Config::GetInstance().Clear();

    // Load from INI file
    if (Config::GetInstance().Load("d:/AppConfig.ini")) {
        RLOG_INFO("Config loaded successfully");

        // Verify loaded values
        RLOG_INFO("Loaded Width: %d", Config::GetInstance().GetInt("Display", "Width"));
        RLOG_INFO("Loaded VSync: %s", (Config::GetInstance().GetBool("Display", "VSync") ? "true" : "false"));
        RLOG_INFO("Loaded Gamma: %f", Config::GetInstance().GetFloat("Display", "Gamma"));
    } else {
        RLOG_ERROR("Failed to load configuration");
    }

    const Reality::DisplayManager displayManager;

    // Get current display information
    const DisplayInfo currentInfo = displayManager.GetCurrentDisplayInfo();
    std::cout << "Current Resolution: " << currentInfo.width << "x" << currentInfo.height
              << " @ " << currentInfo.refreshRate << "Hz" << std::endl;
    std::cout << "HDR Support: " << (currentInfo.isHDRSupported ? "Yes" : "No") << std::endl;
    std::cout << "Pixel Format: " << Reality::DisplayManager::PixelFormatToString(currentInfo.pixelFormat) << std::endl;
    std::cout << "Color Depth: " << currentInfo.colorDepth << " bits" << std::endl;

    if (currentInfo.isHDRSupported) {
        std::cout << "Max Luminance: " << currentInfo.maxLuminance << " nits" << std::endl;
        std::cout << "Min Luminance: " << currentInfo.minLuminance << " nits" << std::endl;
        std::cout << "Max Full Frame Luminance: " << currentInfo.maxFullFrameLuminance << " nits" << std::endl;
    }

    // Get available resolutions
    /*
    const std::vector<Reality::DisplayResolution> resolutions = displayManager.GetAvailableResolutions();
    std::cout << "\nAvailable Resolutions:" << std::endl;
    for (const auto& res : resolutions) {
        std::cout << res.width << "x" << res.height << " @ " << res.refreshRate << "Hz" << std::endl;
    }
    */

    /*
    // Initialize application.
    const auto Application = new RealityApplication();
    Application->Initialize("Reality Engine - Sandbox",1920,1080);

    ShaderManager::GetInstance().LoadShader("cube_ps","/Assets/Shaders/cube.psh",ShaderManager::ShaderType::PIXEL);

    // Create a primitive renderer.
    const auto primitiveRenderer = new PrimitiveRenderer();

    // Create a test camera not final.
    // TODO: Create the camera as a GameObject with a camera component.
    const auto camera = new Camera();
    camera->SetPosition(Vector3f(0, 0, 20));
    camera->LookAt(Vector3f(0, 0, -15.f));
    camera->SetPerspective(60.0f, 0.1f, 100.0f);

    // Create a scene.
    const auto mainScene = new Scene;

    // Create a game object
    const auto gameObject = mainScene->CreateGameObject<Reality::TestGameObject>();

    gameObject->SetName("TestGameObject1");

    mainScene->Initialize();

    while (Application->IsRunning()) {
        Application->Update();

        mainScene->Update(Timer::GetDeltaTime());

        camera->MoveForward(10.0f*Timer::GetDeltaTime());

        camera->Render();

        primitiveRenderer->Render();

        Application->Frame();
    }

    Application->Shutdown();
    */
    RealityWindow window("TEST WINDOW", 1920, 1080);
    window.Show();
    HWND hwnd = window.GetNativeHandle();

    // Create DX12 renderer
    Reality::DX12Renderer renderer(hwnd, 1920, 1080);

    // Main loop
    while (!window.ShouldClose()) {
        window.ProcessMessages();

        // Render frame
        renderer.Render();
    }

    return 0;
}