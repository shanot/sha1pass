/*Copyright (c) 2015 Samuel Hanot



Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:



The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.



THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.*/

#include <gtkmm.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include <algorithm>
#include <functional>
#include <iomanip>
#include <sstream>
#include <vector>

auto get_sha1(const std::string &s) {
  std::vector<unsigned char> d(s.size());
  std::copy(s.cbegin(), s.cend(), d.begin());
  std::vector<unsigned char> md(SHA_DIGEST_LENGTH);
  SHA1(d.data(), d.size(), md.data());
  return md;
}

auto get_hex(const std::vector<unsigned char> &md) {
  std::vector<int> tmp(md.size());
  std::copy(md.begin(), md.end(), tmp.begin());
  std::stringstream out;
  for (const auto &c : tmp) {
    out << std::setfill('0') << std::setw(2) << std::hex << c;
  }
  return out.str();
}

auto get_b64(const std::vector<unsigned char> &md) {
  BIO *b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO *mem = BIO_new(BIO_s_mem());
  BIO_push(b64, mem);
  BIO_write(b64, md.data(), md.size());
  BIO_flush(b64);

  unsigned char *out;
  const auto len = BIO_get_mem_data(mem, &out);

  std::string outs(len, '\0');
  std::copy_n(out, len, outs.begin());

  BIO_free_all(b64);
  return outs;
}

auto get_half(const std::string &in) {
  std::string out(in.size() / 2, '\0');
  std::copy_n(in.begin(), out.size(), out.begin());
  return out;
}

struct Hash {
  std::string name;
  std::function<std::string(std::string)> get;
};

class Sha1pass : public Gtk::Window {
public:
  Sha1pass(const std::vector<Hash> &in) : buttons(in.size()), hash(in) {
    add(grid);
    auto ewidth = buttons.size() / 2;

    auto row = 0;
    grid.attach(salt, 0, row, ewidth, 1);
    grid.attach(key, ewidth, row, ewidth, 1);
    show.set_active(false);
    set_text_visibility(show.get_active());
    secure.set_active(false);

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

    grid.attach(complex, 2, row, 1, 1);
    complex.set_label("complex");

    grid.attach(peek, 3, row, 1, 1);

    ++row;
    for (auto i(0); i < hash.size(); ++i) {
      buttons[i].set_label(hash[i].name);
      buttons[i].signal_clicked().connect({[=]() {
        const auto in = salt.get_text() + key.get_text();
        auto out = hash[i].get(in);
        if (complex.get_active()) {
          out += ".H0k";
        }
        peek.set_text(out.substr(0, 5));
        auto clip = Gtk::Clipboard::get();
        clip->set_text(out);
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
  Gtk::CheckButton complex;
  Gtk::Label peek;
  std::vector<Gtk::Button> buttons;
  std::vector<Hash> hash;

  void clear_text() {
    salt.set_text("");
    key.set_text("");
  }
  void set_text_visibility(const bool &b) {
    salt.set_visibility(b);
    key.set_visibility(b);
  }
};

main() {
  auto app = Gtk::Application::create("org.sha1pass");

  Hash hex{"hex", [](std::string s) { return get_hex(get_sha1(s)); }};
  Hash hex_h{"hex half",
             [](std::string s) { return get_half(get_hex(get_sha1(s))); }};
  Hash b64{"Base64", [](std::string s) { return get_b64(get_sha1(s)); }};
  Hash b64_h{"Base64 half",
             [](std::string s) { return get_half(get_b64(get_sha1(s))); }};

  Sha1pass sha1pass{{hex, hex_h, b64, b64_h}};

  return app->run(sha1pass);
}
