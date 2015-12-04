#include <gtkmm.h>
#include <vector>
#include <sstream>

struct Hash {
  std::string name;
  std::string hashcmd;
};

class Sha1pass : public Gtk::Window {
public:
  void clear_text() {
    salt.set_text("");
    key.set_text("");
  }
  void set_text_visibility(const bool &b) {
    salt.set_visibility(b);
    key.set_visibility(b);
  }
  Sha1pass(const std::vector<Hash> &in) : buttons(in.size()), hash(in) {
    add(grid);
    auto ewidth = buttons.size() / 2;

    auto row = 0;
    grid.attach(salt, 0, row, ewidth, 1);
    grid.attach(key, ewidth, row, ewidth, 1);
    show.set_active(false);
    set_text_visibility(show.get_active());
    secure.set_active(true);

    ++row;
    grid.attach(show, 0, row, 1, 1);
    show.set_label("show");
    show.signal_clicked().connect([&]() {
      if (secure.get_active() && show.get_active()) {
        clear_text();
        secure.set_active(false);
      }
      set_text_visibility(show.get_active());
    });
    grid.attach(secure, 1, row, 1, 1);
    secure.set_label("secure");
    secure.signal_clicked().connect([&]() {
      if (secure.get_active()) {
        show.set_active(false);
        set_text_visibility(show.get_active());
      } else {
        clear_text();
      }
    });

    ++row;
    for (auto i(0); i < hash.size(); ++i) {
      buttons[i].set_label(hash[i].name);
      buttons[i].signal_clicked().connect({[=]() {
        const auto in = salt.get_text() + key.get_text();
        std::stringstream cmd;
        cmd << "echo -n " << in << " | openssl sha1 " << hash[i].hashcmd
            << " | xclip";
        if (system(cmd.str().c_str())) {
          abort();
        }
      }});
      grid.attach(buttons[i], i, row, 1, 1);
    }
    show_all_children();
  }
  virtual ~Sha1pass() {}

private:
  Gtk::Grid grid;
  Gtk::Entry salt;
  Gtk::Entry key;
  Gtk::CheckButton show;
  Gtk::CheckButton secure;
  std::vector<Gtk::Button> buttons;
  std::vector<Hash> hash;
};

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create(argc, argv, "org.sha1pass");

  Hash hex{"HEX", "-hex -r | cut -d ' ' -f 1 | tr -d '\n'"};
  Hash hex_h{"HEX half", "-hex -r | cut -d ' ' -f 1 | head -c 20"};
  Hash b64{"Base64", "-binary | openssl base64  | tr -d '\n'"};
  Hash b64_h{"Base64 half", "-binary | openssl base64  | head -c 14"};

  Sha1pass sha1pass{{hex, hex_h, b64, b64_h}};

  return app->run(sha1pass);
}
