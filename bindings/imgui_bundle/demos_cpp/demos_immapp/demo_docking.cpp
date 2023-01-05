/*
"A more complex app demo,

It demonstrates:
- How to use a specific application state (instead of using static variables)
- How to set up a complex layout:
    - dockable windows that can be moved, and even be detached from the main window
    - status bar
- A default menu, with default
- log window
- How to load assets and fonts"
*/

#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/icons_font_awesome.h"
#include "imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"
#include "immapp/immapp.h"
#include "demo_utils/api_demos.h"

struct AppState {
    float f = 0.0f;
    int counter = 0;
    float rocket_progress = 0.0f;

    enum class RocketState {
        Init,
        Preparing,
        Launched
    };
    RocketState rocket_state = RocketState::Init;
};


// Font loading:
//
// We have two options: either we use hello imgui, or we load manually
// (see MyLoadFontsViaHelloImGui() and MyLoadFontsManually() below).

ImFont* gAkronimFont; // This is just a demo, you should store this somewhere in the app state

void MyLoadFontsViaHelloImGui()
{
    // hello_imgui can load font and merge them with font awesome automatically.
    // It will load them from the assets/ folder.

    // First, we load the default fonts (the font that was loaded first is the default font)
    // LoadDefaultFont_WithFontAwesomeIcons returns a lambda which we need to call, hence the double ()()
    HelloImGui::ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons()();

    // Then we load our custom font
    const std::string fontFilename = "fonts/Akronim-Regular.ttf";
    gAkronimFont = HelloImGui::LoadFontTTF_WithFontAwesomeIcons(fontFilename.c_str(), 40.f);
}


void MyLoadFontsManually()
{
    // Fixme: this version triggers an exception in debug mode under msvc, far later, and deep inside FontAtlas callstack.
    // (although it seems to work fine in release mode. Probable memory overflow somewhere)

    // first, we load the default font (it will not include icons)
    ImGui::GetIO().Fonts->AddFontDefault();

    // Load a font and merge icons into it
    // i. load the font...
    ImFontAtlas* fontAtlas = ImGui::GetIO().Fonts;
    // We need to take into account the global font scale! This is required for macOS retina screens
    const float fontSizePixel = 40.0f / ImGui::GetIO().FontGlobalScale;
    const std::string fontFilename = "demos_assets/fonts/Akronim-Regular.ttf";
    // static is important here (see laius below)
    static const ImWchar* glyphRange = fontAtlas->GetGlyphRangesDefault();
    gAkronimFont = fontAtlas->AddFontFromFileTTF(fontFilename.c_str(), fontSizePixel, NULL, glyphRange);

    // ii. ... And merge icons into the previous font
    // See warning inside imgui.h:
    //     If you pass a 'glyph_ranges' array to AddFont*** functions, you need to make sure that your array persist up until the
    //     atlas is build (when calling GetTexData*** or Build()). We only copy the pointer, not the data.
    // 
    // => We need to make sure that iconRanges is not destroyed when exiting this function! In this case, we can make it either static or constexpr.
    constexpr ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    // Similar warning for fontConfig which is passed by address
    static ImFontConfig fontConfig;
    fontConfig.MergeMode = true;
    gAkronimFont = fontAtlas->AddFontFromFileTTF("demos_assets/fonts/fontawesome-webfont.ttf", fontSizePixel, &fontConfig, iconRanges);
}


void MyLoadFonts()
{
    // Uncomment here your preferred method
    MyLoadFontsViaHelloImGui();
    //MyLoadFontsManually();
}


void CommandGui(AppState& state)
{
    // Note, you can also show the tweak theme widgets via:
    // hello_imgui.show_theme_tweak_gui(hello_imgui.get_runner_params().imgui_window_params.tweaked_theme)
    ImGuiMd::RenderUnindented(R"(
        # Tweak the theme!

        Select the menu "View/Theme/Theme tweak window" in order to browse the available themes (more than 15).
        You can even easily tweak their colors.
    )");

    ImGui::Separator();

    ImGui::PushFont(gAkronimFont);
    ImGui::Text("Hello  %s", ICON_FA_SMILE);
    HelloImGui::ImageFromAsset("world.jpg", ImVec2(100, 100));
    ImGui::PopFont();
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(R"(
        The custom font and the globe image below were loaded
        from the application assets folder
        (those files are embedded automatically).
        )");
    }

    ImGui::Separator();

    // Edit 1 float using a slider from 0.0f to 1.0f
    bool changed = ImGui::SliderFloat("float", &state.f, 0.0f, 1.0f);
    if (changed)
    {
        HelloImGui::Log(HelloImGui::LogLevel::Warning, "state.f was changed to %f", state.f);
    }

    // Buttons return true when clicked (most widgets return true when edited/activated)
    if (ImGui::Button("Button"))
    {
        state.counter++;
        HelloImGui::Log(HelloImGui::LogLevel::Info, "Button was pressed");
    }

    ImGui::SameLine();
    ImGui::Text("counter = %d", state.counter);

    if (state.rocket_state == AppState::RocketState::Init)
    {
        if (ImGui::Button(ICON_FA_ROCKET" Launch rocket"))
        {
            state.rocket_state = AppState::RocketState::Preparing;
            HelloImGui::Log(HelloImGui::LogLevel::Warning, "Rocket is being prepared");
        }
    }
    else if (state.rocket_state == AppState::RocketState::Preparing)
    {
        ImGui::Text("Please Wait");
        state.rocket_progress += 0.003f;
        if (state.rocket_progress >= 1.0f)
        {
            state.rocket_state = AppState::RocketState::Launched;
            HelloImGui::Log(HelloImGui::LogLevel::Warning, "Rocket was launched");
        }
    }
    else if (state.rocket_state == AppState::RocketState::Launched)
    {
        ImGui::Text(ICON_FA_ROCKET " Rocket launched");
        if (ImGui::Button("Reset Rocket"))
        {
            state.rocket_state = AppState::RocketState::Init;
            state.rocket_progress = 0.f;
        }
    }
}


void StatusBarGui(AppState& app_state)
{
    if (app_state.rocket_state == AppState::RocketState::Preparing)
    {
        ImGui::Text("Rocket completion: ");
        ImGui::SameLine();
        ImGui::ProgressBar(app_state.rocket_progress, ImVec2(100.0f, 15.0f));
    }
}


int main()
{
    // Important: HelloImGui uses an assets dir where it can find assets (fonts, images, etc.)
    HelloImGui::SetAssetsFolder(DemosAssetsFolder());

    //###############################################################################################
    // Part 1: Define the application state, fill the status and menu bars, and load additional font
    //###############################################################################################

    // Our application state
    AppState appState;

    // Hello ImGui params (they hold the settings as well as the Gui callbacks)
    HelloImGui::RunnerParams runnerParams;

    runnerParams.appWindowParams.windowTitle = "Docking demo";
    runnerParams.appWindowParams.windowGeometry.size = {1000, 800};
    runnerParams.appWindowParams.restorePreviousGeometry = true;

    //
    // Status bar
    //
    // We use the default status bar of Hello ImGui
    runnerParams.imGuiWindowParams.showStatusBar = true;
    // uncomment next line in order to hide the FPS in the status bar
    // runnerParams.imGuiWindowParams.showStatusFps = false;
    runnerParams.callbacks.ShowStatus = [&appState]() { StatusBarGui(appState); };

    //
    // Menu bar
    //
    // We use the default menu of Hello ImGui, to which we add some more items
    runnerParams.imGuiWindowParams.showMenuBar = true;
    auto ShowMenuGui = []()
    {
        if (ImGui::BeginMenu("My Menu"))
        {
            bool clicked = ImGui::MenuItem("Test me", "", false);
            if (clicked)
            {
                HelloImGui::Log(HelloImGui::LogLevel::Warning, "It works");
            }
            ImGui::EndMenu();
        }
    };
    runnerParams.callbacks.ShowMenus = ShowMenuGui;

    // Choose here your preferred method for loading fonts:
    // runnerParams.callbacks.LoadAdditionalFonts = MyLoadFontsViaHelloImGui;
    runnerParams.callbacks.LoadAdditionalFonts = MyLoadFontsManually;

    // optional native events handling
    // runnerParams.callbacks.AnyBackendEventCallback = ...


    //###############################################################################################
    // Part 2: Define the application layout and windows
    //###############################################################################################

    //
    //    2.1 Define the docking splits,
    //    i.e. the way the screen space is split in different target zones for the dockable windows
    //     We want to split "MainDockSpace" (which is provided automatically) into three zones, like this:
    //
    //    ___________________________________________
    //    |        |                                |
    //    | Left   |                                |
    //    | Space  |    MainDockSpace               |
    //    |        |                                |
    //    |        |                                |
    //    |        |                                |
    //    -------------------------------------------
    //    |     BottomSpace                         |
    //    -------------------------------------------
    //

    // First, tell HelloImGui that we want full screen dock space (this will create "MainDockSpace")
    runnerParams.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
    // In this demo, we also demonstrate multiple viewports.
    // you can drag windows outside out the main window in order to put their content into new native windows
    runnerParams.imGuiWindowParams.enableViewports = true;

    // Then, add a space named "BottomSpace" whose height is 25% of the app height.
    // This will split the preexisting default dockspace "MainDockSpace" in two parts.
    HelloImGui::DockingSplit splitMainBottom;
    splitMainBottom.initialDock = "MainDockSpace";
    splitMainBottom.newDock = "BottomSpace";
    splitMainBottom.direction = ImGuiDir_Down;
    splitMainBottom.ratio = 0.25f;

    // Then, add a space to the left which occupies a column whose width is 25% of the app width
    HelloImGui::DockingSplit splitMainLeft;
    splitMainLeft.initialDock = "MainDockSpace";
    splitMainLeft.newDock = "LeftSpace";
    splitMainLeft.direction = ImGuiDir_Left;
    splitMainLeft.ratio = 0.25f;

    // Finally, transmit these splits to HelloImGui
    runnerParams.dockingParams.dockingSplits = {splitMainBottom, splitMainLeft};

    //
    // 2.1 Define our dockable windows : each window provide a Gui callback, and will be displayed
    //     in a docking split.
    //
    HelloImGui::DockableWindow commandsWindow;
    commandsWindow.label = "Commands";
    commandsWindow.dockSpaceName = "LeftSpace";
    commandsWindow.GuiFunction = [&]{ CommandGui(appState); };


    // A Log window named "Logs" will be placed in "BottomSpace". It uses the HelloImGui logger gui
    HelloImGui::DockableWindow logsWindow;
    logsWindow.label = "Logs";
    logsWindow.dockSpaceName = "BottomSpace";
    logsWindow.GuiFunction = [] { HelloImGui::LogGui(); };
    // A Window named "Dear ImGui Demo" will be placed in "MainDockSpace"
    HelloImGui::DockableWindow dearImGuiDemoWindow;
    dearImGuiDemoWindow.label = "Dear ImGui Demo";
    dearImGuiDemoWindow.dockSpaceName = "MainDockSpace";
    dearImGuiDemoWindow.GuiFunction = [] { ImGui::ShowDemoWindow(); };

    // Finally, transmit these windows to HelloImGui
    runnerParams.dockingParams.dockableWindows = {
        commandsWindow,
        logsWindow,
        dearImGuiDemoWindow,
    };

    //###############################################################################################
    // Part 3: Run the app
    //###############################################################################################
    ImmApp::AddOnsParams addonsParams;
    addonsParams.withMarkdown = true;
    ImmApp::Run(runnerParams, addonsParams);
}
