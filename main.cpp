#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <thread>
#include <mutex>

using namespace ftxui;

int main() {
  auto screen = ScreenInteractive::TerminalOutput();

  int selected_region = 0;
  std::vector<std::string> region_options = {
    "国服 (cn)",
    "国际服 (gl)",
    "日服 (jp)"
  };
  std::vector<std::string> region_values = {"cn", "gl", "jp"};

  std::string threads;        // --threads / -t（默认值 20 参考，但留空表示不调用）
  std::string version;        // --version / -v（无默认值）
  std::string raw;            // --raw / -r（默认值 "RawData" 参考）
  std::string extract;        // --extract / -e（默认值 "Extracted" 参考）
  std::string temporary;      // --temporary / -m（默认值 "Temp" 参考）
  bool downloading_extract = false; // --downloading-extract / -de
  std::string resource_type;  // --resource-type / -rt（默认值 all 参考）
  std::string proxy;          // --proxy / -p（无默认值）
  std::string max_retries;    // --max-retries / -mr（默认值 5 参考）
  std::string search;         // --search / -s
  std::string advance_search; // --advance-search / -as

  std::string command_output;
  std::mutex output_mutex;

  Component region_radiobox = Radiobox(&region_options, &selected_region);

  Component threads_input = Input(&threads, "请输入线程数 (默认20)：");
  Component version_input = Input(&version, "请输入资源版本号 (仅国际服务器)：");
  Component raw_input = Input(&raw, "请输入未处理文件路径 (默认 RawData)：");
  Component extract_input = Input(&extract, "请输入已提取文件路径 (默认 Extracted)：");
  Component temporary_input = Input(&temporary, "请输入临时文件路径 (默认 Temp)：");
  Component downloading_extract_checkbox = Checkbox("下载时提取（较慢，资源数量多于500时请勿使用）", &downloading_extract);
  Component resource_type_input = Input(&resource_type, "请输入资源类型 (table, media, bundle, all；默认 all)：");
  Component proxy_input = Input(&proxy, "请输入HTTP代理 (例如 http://127.0.0.1:8080)：");
  Component max_retries_input = Input(&max_retries, "请输入最大重试次数 (默认5)：");
  Component search_input = Input(&search, "请输入普通检索关键词（多个请以空格分隔）：");
  Component advance_search_input = Input(&advance_search, "请输入高级检索角色关键字（多个请以空格分隔）：");

  bool is_running = false;

  Component run_button = Button("运行", [&] {
    if (is_running)
      return;
    is_running = true;

    std::ostringstream cmd;
    cmd << "python ./scripts/main.py";
    cmd << " -g " << region_values[selected_region];

    if (!threads.empty())
      cmd << " -t " << threads;
    if (!version.empty())
      cmd << " -v " << version;
    if (!raw.empty())
      cmd << " -r " << raw;
    if (!extract.empty())
      cmd << " -e " << extract;
    if (!temporary.empty())
      cmd << " -m " << temporary;
    if (downloading_extract)
      cmd << " -de";
    if (!resource_type.empty())
      cmd << " -rt " << resource_type;
    if (!proxy.empty())
      cmd << " -p " << proxy;
    if (!max_retries.empty())
      cmd << " -mr " << max_retries;
    if (!search.empty())
      cmd << " -s " << search;
    if (!advance_search.empty())
      cmd << " -as " << advance_search;

    std::string command = cmd.str();
    {
      std::lock_guard<std::mutex> lock(output_mutex);
      command_output = "执行命令: " + command;
    }

    std::string terminal_command;
#ifdef _WIN32
    terminal_command = "start cmd /k " + command;
#else
    terminal_command = "xterm -hold -e \"" + command + "\" &";
#endif

    std::thread([&, terminal_command] {
      system(terminal_command.c_str());
      {
        std::lock_guard<std::mutex> lock(output_mutex);
        command_output += "\n新终端窗口关闭。";
      }
      is_running = false;
    }).detach();
  });

  Component quit_button = Button("退出", [&] {
    screen.Exit();
  });

  auto output_renderer = Renderer([&] {
    std::lock_guard<std::mutex> lock(output_mutex);
    return vbox({ text(command_output) }) | border;
  });

  auto container = Container::Vertical({
      region_radiobox,
      threads_input,
      version_input,
      raw_input,
      extract_input,
      temporary_input,
      downloading_extract_checkbox,
      resource_type_input,
      proxy_input,
      max_retries_input,
      search_input,
      advance_search_input,
      Container::Horizontal({
          run_button,
          quit_button,
      }),
      output_renderer,
  });

  auto renderer = Renderer(container, [&] {
    return vbox({
      text("Blue Archive Asset Downloader"),
      separator(),
      hbox(text("请选择服务器区域: "), region_radiobox->Render()),
      threads_input->Render(),
      version_input->Render(),
      raw_input->Render(),
      extract_input->Render(),
      temporary_input->Render(),
      downloading_extract_checkbox->Render(),
      resource_type_input->Render(),
      proxy_input->Render(),
      max_retries_input->Render(),
      search_input->Render(),
      advance_search_input->Render(),
      hbox({
        run_button->Render(),
        text("   "),
        quit_button->Render(),
      }),
      separator(),
      text("状态:"),
      output_renderer->Render(),
    }) | border;
  });

  screen.Loop(renderer);
  return 0;
}