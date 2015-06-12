#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <future>
// #include <queue>

#include <async_https_json_stream.hpp>

#include <liboauthcpp/liboauthcpp.h>

#include <picojson.h>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/OpenGL.hpp>

std::string tw_stream_host = "stream.twitter.com";
std::string tw_stream_port = "443";
std::string tw_stream_endpoint = "/1.1/statuses/filter.json";
std::string tw_stream_url = "https://" + tw_stream_host + tw_stream_endpoint;
std::string tw_term;

bool display_running = true;
std::mutex mtx;

// auto imagelist = std::queue<sf::Image>{};
int viewportWidth = 800;
int viewportHeight = 600;
std::string windowTitle = "twitter image wall";

const int IMG_COUNT = 4;
sf::Image image[IMG_COUNT];
sf::Vector2u imageSize[IMG_COUNT];
sf::Texture texture[IMG_COUNT];
sf::Sprite sprite[IMG_COUNT];

boost::asio::io_service io_service;
boost::asio::ssl::context io_context(boost::asio::ssl::context::sslv23);

void fetch_image(const std::string& url) {
  std::cout << url << '\n';

  // find the pivot point to split host and path components
  std::size_t pivot = url.find("://");
  if (pivot != std::string::npos) {
    pivot = url.find('/', pivot+4);
  }

  if (pivot == std::string::npos) {
    return;
  }

  auto host_part = url.substr(0, pivot);
  auto path_part = url.substr(pivot);

  // std::cout << "host: " << host_part
  //   << " path: " << path_part
  //   << " pivot: " << pivot
  //   << '\n';

  sf::Http http;
  http.setHost(host_part);

  sf::Http::Request::Request req(path_part);
  auto res = http.sendRequest(req);
  auto imageData = res.getBody();

  {
    std::unique_lock<std::mutex> lck(mtx);

    for (int i=IMG_COUNT-1; i!=0; --i) {
      sprite[i] = sprite[i-1];
      texture[i] = texture[i-1];
      image[i] = image[i-1];
      imageSize[i] = imageSize[i-1];

      texture[i].update(image[i]);
      sprite[i].setTexture(texture[i]);
    }

    image[0] = sf::Image{};
    image[0].loadFromMemory(imageData.c_str(), imageData.length());

    imageSize[0] = image[0].getSize();

    texture[0] = sf::Texture{};
    texture[0].loadFromImage(image[0]);

    sprite[0] = sf::Sprite{};
    sprite[0].setTexture(texture[0]);

    // sprite[0].setOrigin(sf::Vector2f(imageSize[0].x / 2.f, imageSize[0].y / 2.f));
    sprite[0].setOrigin(sf::Vector2f(0, imageSize[0].y * .5f));
    sprite[1].setOrigin(sf::Vector2f(0, 0));
    sprite[2].setOrigin(sf::Vector2f(0, 0));
    sprite[3].setOrigin(sf::Vector2f(0, 0));

    sprite[0].setPosition(sf::Vector2f(0, viewportHeight * .5f));
    sprite[1].setPosition(sf::Vector2f(viewportWidth * .5f, 0));
    sprite[2].setPosition(sf::Vector2f(viewportWidth * .5f, viewportHeight * .3f));
    sprite[3].setPosition(sf::Vector2f(viewportWidth * .5f, viewportHeight * .6f));

    sprite[0].setScale(
      sf::Vector2f(
        ((float) viewportWidth  * 0.5f) / (float) imageSize[0].x,
        ((float) viewportHeight * 1.0f) / (float) imageSize[0].y
      )
    );

    for (int i=1; i!=IMG_COUNT; ++i) {
      sprite[i].setScale(
        sf::Vector2f(
          ((float) viewportWidth  * 0.5f) / (float) imageSize[i].x,
          ((float) viewportHeight * 0.3f) / (float) imageSize[i].y
        )
      );
    }
    // sprite[1].setScale(sf::Vector2f(.5f, .5f));
    // sprite[2].setScale(sf::Vector2f(.5f, .5f));
    // sprite[3].setScale(sf::Vector2f(.5f, .5f));
  }
}

void extract_image_urls(picojson::value& v)
{
  if (v.is<picojson::object>()) {
    const auto& t = v.get<picojson::object>();

    if (std::end(t) != t.find("entities")) {
      const auto& entities = t.at("entities");

      if (entities.is<picojson::object>()) {
        const auto& entities_obj = entities.get<picojson::object>();

        if (std::end(entities_obj) != entities_obj.find("media")) {
          const auto& media = entities_obj.at("media");

          if (media.is<picojson::array>()) {
            const auto& media_list = media.get<picojson::array>();

            for (auto& m : media_list) {

              if (m.is<picojson::object>()) {
                const auto& mo = m.get<picojson::object>();

                const auto& media_url = mo.at("media_url");
                if (media_url.is<std::string>()) {
                  std::async(fetch_image, media_url.to_str());
                }

              }

            }

          }
        }
      }
    }
  }
}

void setup_network_stream()
{
  std::string consumer_key = std::getenv("tw_consumer_key");
  std::string consumer_secret = std::getenv("tw_consumer_secret");
  std::string oauth_token = std::getenv("tw_oauth_token");
  std::string oauth_token_secret = std::getenv("tw_oauth_token_secret");

  std::string tw_stream_params = "track=" + tw_term;

  OAuth::Consumer consumer(consumer_key, consumer_secret);
  OAuth::Token token(oauth_token, oauth_token_secret);
  OAuth::Client oauth(&consumer, &token);

  std::string oAuthHeader =
      oauth.getHttpHeader(
          OAuth::Http::Post,
          tw_stream_url,
          tw_stream_params);

  int tweet_max = 13;
  int tweet_count;

  ahjs::AsyncHttpsJsonStream c(
      io_service,
      io_context,
      tw_stream_host,
      tw_stream_port,
      tw_stream_endpoint,
      oAuthHeader,
      tw_stream_params,
      [&tweet_max, &tweet_count] (const std::string& json) {
        // std::cout << json << '\n';

        picojson::value v;
        std::string err = picojson::parse(v, json);
        if (!err.empty()) {
          std::cerr << err << std::endl;
        } else {
          extract_image_urls(v);
        }
      }
  );

  io_service.run();
}

void draw_window(bool full_screen)
{
  auto desktop = sf::VideoMode::getDesktopMode();

  if (full_screen) {
    viewportWidth = desktop.width;
    viewportHeight = desktop.height;
  }

  sf::RenderWindow window(
    sf::VideoMode(viewportWidth, viewportHeight),
    windowTitle,
    (full_screen) ? sf::Style::Fullscreen : sf::Style::Default
  );

  window.setVerticalSyncEnabled(true);

  // load resources, initialize the OpenGL states, ...
  glEnable(GL_TEXTURE_2D);

  {
    std::unique_lock<std::mutex> lck(mtx);
    image[0].loadFromFile("loading.jpg");
    imageSize[0] = image[0].getSize();
    texture[0].loadFromImage(image[0]);
    sprite[0].setTexture(texture[0]);
    sprite[0].setOrigin(sf::Vector2f(imageSize[0].x / 2.f, imageSize[0].y / 2.f));
    sprite[0].setPosition(sf::Vector2f(viewportWidth / 2.f, viewportHeight / 2.f));
  }

  while (display_running)
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
      {
        display_running = false;
      }
      else if (event.type == sf::Event::Resized)
      {
        // adjust the viewport when the window is resized
        viewportWidth = event.size.width;
        viewportHeight = event.size.height;
        // sprite.setPosition(sf::Vector2f(viewportWidth / 2.f, viewportHeight / 2.f));
        glViewport(0, 0, viewportWidth, viewportHeight);
      }
    }

    // clear the buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw
    {
      std::unique_lock<std::mutex> lck(mtx);

      for (int i=0; i!=IMG_COUNT; ++i)
        window.draw(sprite[i]);
    }

    // end the current frame (internally swaps the front and back buffers)
    window.display();
  }
}

int main(int argc, char** argv)
{
  if (2 > argc) {
    std::cout << "\nusage: " << argv[0] << " <term to track> [--full_screen]\n\n";
    return 1;
  }

  tw_term = argv[1];
  std::thread t1(setup_network_stream);

  draw_window(3 == argc);

  io_service.stop();
  t1.detach();

  return 0;
}