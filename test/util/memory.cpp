#include <mbgl/test/util.hpp>
#include <mbgl/test/stub_file_source.hpp>

#include <mbgl/map/map.hpp>
#include <mbgl/platform/default/headless_display.hpp>
#include <mbgl/platform/default/headless_view.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/run_loop.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

#include <unistd.h>

using namespace mbgl;
using namespace std::literals::string_literals;

long getRSS() {
#ifdef __linux__
    auto statm = util::read_file("/proc/self/statm");

    std::vector<std::string> stats;
    std::istringstream stream(statm);

    std::copy(std::istream_iterator<std::string>(stream),
        std::istream_iterator<std::string>(),
        std::back_inserter(stats));

    return std::stol(stats[1]) * getpagesize();
#else
    return 0L;
#endif
}

class MemoryTest {
public:
    MemoryTest() {
        fileSource.styleResponse = [&](const Resource& res) { return response("style_" + getType(res) + ".json");};
        fileSource.tileResponse = [&](const Resource& res) { return response(getType(res) + ".tile"); };
        fileSource.sourceResponse = [&](const Resource& res) { return response("source_" + getType(res) + ".json"); };
        fileSource.glyphsResponse = [&](const Resource&) { return response("glyphs.pbf"); };
        fileSource.spriteJSONResponse = [&](const Resource&) { return response("sprite.json"); };
        fileSource.spriteImageResponse = [&](const Resource&) { return response("sprite.png"); };
    }

    util::RunLoop runLoop;
    std::shared_ptr<HeadlessDisplay> display { std::make_shared<mbgl::HeadlessDisplay>() };
    HeadlessView view { display, 1 };
    StubFileSource fileSource;

private:
    Response response(const std::string& path) {
        Response result;
        result.data = std::make_shared<std::string>(util::read_file("test/fixtures/resources/"s + path));
        return result;
    }

    std::string getType(const Resource& res) {
        if (res.url.find("satellite") != std::string::npos) {
            return "raster";
        } else {
            return "vector";
        }
    };
};

TEST(Memory, Vector) {
    MemoryTest test;

    Map map(test.view, test.fileSource, MapMode::Still);
    map.setZoom(16); // more map features
    map.setStyleURL("mapbox://streets");

    test::render(map);
    test.runLoop.runOnce();
}

TEST(Memory, Raster) {
    MemoryTest test;

    Map map(test.view, test.fileSource, MapMode::Still);
    map.setStyleURL("mapbox://satellite");

    test::render(map);
    test.runLoop.runOnce();
}

TEST(Memory, Load) {
    MemoryTest test;

    Map map(test.view, test.fileSource, MapMode::Still);
    map.setZoom(16);

    auto renderRaster = [&] {
        map.setStyleURL("mapbox://satellite");
        test::render(map);
        test.runLoop.runOnce();
    };

    auto renderVector = [&] {
        map.setStyleURL("mapbox://streets");
        test::render(map);
        test.runLoop.runOnce();
    };

    // Warm up buffers.
    for (unsigned i = 0; i < 50; ++i) {
        renderRaster();
        renderVector();
    }

    long startRSS = getRSS();

    for (unsigned i = 0; i < 100; ++i) {
        renderRaster();
        renderVector();
    }

    auto deltaRSS = getRSS() - startRSS;

    ASSERT_LT(deltaRSS, 10 * 1024 * 1024) << "\
        Abnormal memory growth detected.";
}
