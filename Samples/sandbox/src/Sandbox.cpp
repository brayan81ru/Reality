#include <Reality.h>

int main() {
    // Initialize log
    // Configure logging
    Log::GetInstance().SetLevel(Reality::LogLevel::Debug);
    Log::GetInstance().SetLogFile("engine.log");
    Log::GetInstance().EnableFileOutput(true);
    Log::GetInstance().EnableColors(true);

    // Log messages at different levels
    RLOG_TRACE("This is a trace message");
    RLOG_DEBUG("Debug information");
    RLOG_INFO("Application started successfully");
    RLOG_WARNING("This is a warning");
    auto errorCode = 404;
    RLOG_ERROR("An error occurred! %dx%d",errorCode,errorCode);
    RLOG_FATAL("Critical failure!");

    // Using the class directly
    Log::GetInstance().Info("Direct access to logger");

    // Initialize config.
    Config AppConfig;
    AppConfig.Set("Display", "Width", "1920");
    AppConfig.Set("Display", "Height", "1080");
    AppConfig.Set("Display", "VSync", "true");
    AppConfig.Set("Display", "Gamma", "0.5");
    AppConfig.Set("Audio", "Volume", "0.8");
    AppConfig.Set("Audio", "MusicEnabled", "true");

    // Get values as different types
    const std::string widthStr = AppConfig.Get("Display", "Width");
    const int width = AppConfig.GetInt("Display", "Width");
    const bool vsync = AppConfig.GetBool("Display", "VSync");
    const float gamma = AppConfig.GetFloat("Display", "Gamma");

    std::cout << "Width (string): " << widthStr << "\n";
    std::cout << "Width (int): " << width << "\n";
    std::cout << "VSync: " << (vsync ? "true" : "false") << "\n";
    std::cout << "Gamma: " << gamma << "\n";

    // Save to INI file
    if (AppConfig.Save("d:/AppConfig.ini")) {
        std::cout << "Configuration saved successfully!\n";
    } else {
        std::cout << "Failed to save configuration!\n";
    }

    // Clear current configuration
    AppConfig.Clear();

    // Load from INI file
    if (AppConfig.Load("d:/AppConfig.ini")) {
        std::cout << "Configuration loaded successfully!\n";

        // Verify loaded values
        std::cout << "Loaded Width: " << AppConfig.GetInt("Display", "Width") << "\n";
        std::cout << "Loaded VSync: " << (AppConfig.GetBool("Display", "VSync") ? "true" : "false") << "\n";
        std::cout << "Loaded Gamma: " << AppConfig.GetFloat("Display", "Gamma") << "\n";
    } else {
        std::cout << "Failed to load configuration!\n";
    }

    // Initialize application.
    RealityApplication Application("Reality Engine - Sandbox",1280,720);

    // Create a primitive renderer.
    const auto primitiveRenderer = new PrimitiveRenderer(Application.GetRenderer());

    // Initialize camera with default settings
    const auto camera = new Camera();

    // Create a camera.
    camera->SetPosition(float3(0, 1, -5));
    camera->LookAt(float3(0, 0, 0));
    camera->SetPerspective(60.0f, 16.0f / 9.0f, 0.1f, 100.0f);

    while (Application.IsRunning()) {
        Application.Update();

        primitiveRenderer->Render(camera);

        Application.Frame();
    }

    Application.Shutdown();

    return 0;
}
