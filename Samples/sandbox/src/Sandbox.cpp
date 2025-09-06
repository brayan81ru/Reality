#include <Reality.h>
#include "imgui.h"
#include "TestGameobject.h"

int main() {
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

    // Initialize application.
    const auto Application = new RealityApplication();
    Application->Initialize("Reality Engine - Sandbox",1920,1080);

    // Create a primitive renderer.
    const auto primitiveRenderer = new PrimitiveRenderer();

    // Initialize camera with default settings
    const auto camera = new Camera();

    // Create a camera.
    camera->SetPosition(Vector3f(0, 0, 5));
    camera->LookAt(Vector3f(0, 0, -15.f));
    camera->SetPerspective(60.0f, 0.1f, 100.0f);

    // Create a scene.
    Scene mainScene;

    // Create a game object
    const auto gameObject = mainScene.CreateGameObject<Reality::TestGameObject>();
    gameObject->SetName("TestGameObject");

    mainScene.Initialize();

    while (Application->IsRunning()) {
        Application->Update();

        mainScene.Update(Timer::GetDeltaTime());

        camera->MoveForward(2.0f*Timer::GetDeltaTime());

        camera->Render();

        primitiveRenderer->Render();

        Application->Frame();
    }

    Application->Shutdown();

    return 0;
}
