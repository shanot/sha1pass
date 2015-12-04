#include <gtkmm.h>
#include <vector>
#include <sstream>

struct Hash {
  std::string name;
  std::string hashcmd;
};

class Sha1pass : public Gtk::Window {
public:
  Sha1pass(const std::vector<Hash>& in) : buttons(in.size()), hash(in) {
    add(grid);
    auto ewidth = buttons.size() / 2;

    grid.attach(salt, 0, 0, ewidth, 1);
    grid.attach(key, ewidth, 0, ewidth, 1);

    for (auto i(0); i < hash.size(); ++i) {
      buttons[i].set_label(hash[i].name);
      buttons[i].signal_clicked().connect({[=]() {
        const auto in = salt.get_text() + key.get_text();
        std::stringstream cmd;
        cmd << "echo -n " << in << " | openssl sha1 " << hash[i].hashcmd << " | xclip";
        if (system(cmd.str().c_str())) {
          abort();
        }
      }});
      grid.attach(buttons[i], i, 1, 1, 1);
    }
    show_all_children();
  }
  virtual ~Sha1pass() {}

private:
  Gtk::Grid grid;
  Gtk::Entry salt;
  Gtk::Entry key;
  std::vector<Gtk::Button> buttons;
  std::vector<Hash> hash;
};

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create(argc, argv, "org.sha1pass");

  Hash hex{"HEX",           "-hex -r | cut -d ' ' -f 1 | tr -d '\n'"};
  Hash hex_h{"HEX half",    "-hex -r | cut -d ' ' -f 1 | head -c 20"};
  Hash b64{"Base64",        "-binary | openssl base64  | tr -d '\n'"};
  Hash b64_h{"Base64 half", "-binary | openssl base64  | head -c 14"};

  Sha1pass sha1pass{{hex, hex_h, b64, b64_h}};

  return app->run(sha1pass);
}
