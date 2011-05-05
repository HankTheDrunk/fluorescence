
#include "manager.hpp"

#include "tiledataloader.hpp"
#include "artloader.hpp"
#include "huesloader.hpp"
#include "gumpartloader.hpp"
#include "maploader.hpp"
#include "staticsloader.hpp"
#include "maptexloader.hpp"
#include "animdataloader.hpp"
#include "animloader.hpp"

#include <ui/singletextureprovider.hpp>
#include <ui/animdatatextureprovider.hpp>

namespace uome {
namespace data {

Manager* Manager::singleton_= NULL;

bool Manager::create(Config& config) {
    if (!singleton_) {
        try {
            singleton_ = new Manager();
            singleton_->init(config);
        } catch (const std::exception& ex) {
            LOGARG_CRITICAL(LOGTYPE_DATA, "Error initializing data::Manager: %s", ex.what());
            return false;
        }
    }

    return true;
}

void Manager::destroy() {
    if (singleton_) {
        delete singleton_;
        singleton_ = NULL;
    }
}

Manager* Manager::getSingleton() {
    if (!singleton_) {
        throw Exception("uome::data::Manager Singleton not created yet");
    }

    return singleton_;
}

Manager::Manager() {
}

void Manager::init(Config& config) {
    boost::filesystem::path mulDirPath = config["/uome/files/mul-directory@path"].asPath();
    if (!boost::filesystem::exists(mulDirPath) || !boost::filesystem::is_directory(mulDirPath)) {
        throw Exception("Invalid mul directory");
    }

    boost::filesystem::path idxPath;
    boost::filesystem::path path;
    boost::filesystem::path difOffsetsPath;
    boost::filesystem::path difIdxPath;
    boost::filesystem::path difPath;

    path = getPathFor(config, "/uome/files/tiledata@filename");
    LOGARG_INFO(LOGTYPE_DATA, "Opening tiledata from %s", path.string().c_str());
    tileDataLoader_.reset(new TileDataLoader(path));

    path = getPathFor(config, "/uome/files/hues@filename");
    LOGARG_INFO(LOGTYPE_DATA, "Opening hues from %s", path.string().c_str());
    huesLoader_.reset(new HuesLoader(path));

    idxPath = getPathFor(config, "/uome/files/texmaps@filename-idx");
    path = getPathFor(config, "/uome/files/texmaps@filename-mul");
    LOGARG_INFO(LOGTYPE_DATA, "Opening maptex from idx=%s mul=%s", idxPath.string().c_str(), path.string().c_str());
    mapTexLoader_.reset(new MapTexLoader(idxPath, path));

    idxPath = getPathFor(config, "/uome/files/art@filename-idx");
    path = getPathFor(config, "/uome/files/art@filename-mul");
    LOGARG_INFO(LOGTYPE_DATA, "Opening art from idx=%s mul=%s", idxPath.string().c_str(), path.string().c_str());
    artLoader_.reset(new ArtLoader(idxPath, path));

    idxPath = getPathFor(config, "/uome/files/gumpart@filename-idx");
    path = getPathFor(config, "/uome/files/gumpart@filename-mul");
    LOGARG_INFO(LOGTYPE_DATA, "Opening gump art from idx=%s mul=%s", idxPath.string().c_str(), path.string().c_str());
    gumpArtLoader_.reset(new GumpArtLoader(idxPath, path));

    path = getPathFor(config, "/uome/files/animdata@filename");
    LOGARG_INFO(LOGTYPE_DATA, "Opening animdata from mul=%s", path.string().c_str());
    animDataLoader_.reset(new AnimDataLoader(path));


    char* mapConfigEnabled = strdup("/uome/files/map0@enabled");
    char* mapConfigMulPath = strdup("/uome/files/map0@filename");
    char* mapConfigSizeX = strdup("/uome/files/map0@width");
    char* mapConfigSizeY = strdup("/uome/files/map0@height");
    char* mapConfigDifEnabled = strdup("/uome/files/map0/difs@enabled");
    char* mapConfigDifOffsetsPath = strdup("/uome/files/map0/difs@filename-offsets");
    char* mapConfigDifMulPath = strdup("/uome/files/map0/difs@filename-mul");

    char* staticsConfigIdxPath = strdup("/uome/files/map0/statics@filename-idx");
    char* staticsConfigMulPath = strdup("/uome/files/map0/statics@filename-mul");
    char* staticsConfigDifEnabled = strdup("/uome/files/map0/statics/difs@enabled");
    char* staticsConfigDifOffsetsPath = strdup("/uome/files/map0/statics/difs@filename-offsets");
    char* staticsConfigDifIdxPath = strdup("/uome/files/map0/statics/difs@filename-idx");
    char* staticsConfigDifMulPath = strdup("/uome/files/map0/statics/difs@filename-mul");

    unsigned int blockCountX;
    unsigned int blockCountY;

    char indexChar = '0';
    unsigned int digitIndex = 15;
    for (unsigned int index = 0; index < 5; ++index, ++indexChar) {
        mapConfigEnabled[digitIndex] = indexChar;

        if (!config[mapConfigEnabled].asBool()) {
            continue;
        }

        mapConfigMulPath[digitIndex] = indexChar;
        mapConfigSizeX[digitIndex] = indexChar;
        mapConfigSizeY[digitIndex] = indexChar;
        mapConfigDifEnabled[digitIndex] = indexChar;
        mapConfigDifOffsetsPath[digitIndex] = indexChar;
        mapConfigDifMulPath[digitIndex] = indexChar;

        staticsConfigIdxPath[digitIndex] = indexChar;
        staticsConfigMulPath[digitIndex] = indexChar;
        staticsConfigDifEnabled[digitIndex] = indexChar;
        staticsConfigDifOffsetsPath[digitIndex] = indexChar;
        staticsConfigDifIdxPath[digitIndex] = indexChar;
        staticsConfigDifMulPath[digitIndex] = indexChar;

        path = getPathFor(config, mapConfigMulPath);
        blockCountX = config[mapConfigSizeX].asInt();
        blockCountY = config[mapConfigSizeY].asInt();
        if (boost::filesystem::exists(path)) {
            if (config[mapConfigDifEnabled].asBool()) {
                difOffsetsPath = getPathFor(config, mapConfigDifOffsetsPath);
                difPath = getPathFor(config, mapConfigDifMulPath);
                LOGARG_INFO(LOGTYPE_DATA, "Opening map%c from mul=%s, dif-offsets=%s, dif=%s, blockCountX=%u, blockCountY=%u", indexChar, path.string().c_str(), difOffsetsPath.string().c_str(), difPath.string().c_str(), blockCountX, blockCountY);
                mapLoader_[index].reset(new MapLoader(path, difOffsetsPath, difPath, blockCountX, blockCountY));
            } else {
                LOGARG_INFO(LOGTYPE_DATA, "Opening map%c from mul=%s, difs disabled, blockCountX=%u, blockCountY=%u", indexChar, path.string().c_str(), blockCountX, blockCountY);
                mapLoader_[index].reset(new MapLoader(path, blockCountX, blockCountY));
            }

            if (!fallbackMapLoader_.get()) {
                fallbackMapLoader_ = mapLoader_[index];
            }
        } else {
            LOGARG_INFO(LOGTYPE_DATA, "Unable to open map%c from mul=%s", indexChar, path.string().c_str());
        }

        idxPath = getPathFor(config, staticsConfigIdxPath);
        path = getPathFor(config, staticsConfigMulPath);

        if (boost::filesystem::exists(idxPath) && boost::filesystem::exists(path)) {
            if (config[staticsConfigDifEnabled].asBool()) {
                difOffsetsPath = getPathFor(config, staticsConfigDifOffsetsPath);
                difIdxPath = getPathFor(config, staticsConfigDifIdxPath);
                difPath = getPathFor(config, staticsConfigDifMulPath);
                LOGARG_INFO(LOGTYPE_DATA, "Opening statics%c from idx=%s, mul=%s, dif-offsets=%s, dif-idx=%s, dif=%s",
                        indexChar, idxPath.string().c_str(), path.string().c_str(),
                        difOffsetsPath.string().c_str(), difIdxPath.string().c_str(), difPath.string().c_str());
                staticsLoader_[index].reset(new StaticsLoader(idxPath, path, difOffsetsPath, difIdxPath, difPath, blockCountX, blockCountY));
            } else {
                LOGARG_INFO(LOGTYPE_DATA, "Opening statics%c from idx=%s, mul=%s, difs disabled,", indexChar, idxPath.string().c_str(), path.string().c_str());
                staticsLoader_[index].reset(new StaticsLoader(idxPath, path, blockCountX, blockCountY));
            }

            if (!fallbackStaticsLoader_.get()) {
                fallbackStaticsLoader_ = staticsLoader_[index];
            }
        } else {
            LOGARG_WARN(LOGTYPE_DATA, "Unable to open statics%c from idx=%s, mul=%s", indexChar, idxPath.string().c_str(), path.string().c_str());
        }
    }

    free(mapConfigEnabled);
    free(mapConfigMulPath);
    free(mapConfigSizeX);
    free(mapConfigSizeY);
    free(mapConfigDifEnabled);
    free(mapConfigDifOffsetsPath);
    free(mapConfigDifMulPath);
    free(staticsConfigIdxPath);
    free(staticsConfigMulPath);
    free(staticsConfigDifEnabled);
    free(staticsConfigDifOffsetsPath);
    free(staticsConfigDifIdxPath);
    free(staticsConfigDifMulPath);



    char* animConfigEnabled = strdup("/uome/files/anim0@enabled");
    char* animConfigIdxPath = strdup("/uome/files/anim0@filename-idx");
    char* animConfigMulPath = strdup("/uome/files/anim0@filename-mul");
    char* animConfigHighDetailCount = strdup("/uome/files/anim0@highdetail");
    char* animConfigLowDetailCount = strdup("/uome/files/anim0@lowdetail");

    unsigned int highDetailCount;
    unsigned int lowDetailCount;

    indexChar = '0';
    digitIndex = 16;
    for (unsigned int index = 0; index < 5; ++index, ++indexChar) {
        animConfigEnabled[digitIndex] = indexChar;

        if (!config[animConfigEnabled].asBool()) {
            continue;
        }

        animConfigIdxPath[digitIndex] = indexChar;
        animConfigMulPath[digitIndex] = indexChar;
        animConfigHighDetailCount[digitIndex] = indexChar;
        animConfigLowDetailCount[digitIndex] = indexChar;

        idxPath = getPathFor(config, animConfigIdxPath);
        path = getPathFor(config, animConfigMulPath);
        highDetailCount = config[animConfigHighDetailCount].asInt();
        lowDetailCount = config[animConfigLowDetailCount].asInt();

        if (boost::filesystem::exists(idxPath) && boost::filesystem::exists(path)) {
            LOGARG_INFO(LOGTYPE_DATA, "Opening anim%c from idx=%s, mul=%s, high detail=%u, low detail=%u", indexChar, idxPath.string().c_str(), path.string().c_str(), highDetailCount, lowDetailCount);
            animLoader_[index].reset(new AnimLoader(idxPath, path, highDetailCount, lowDetailCount));

            if (!fallbackAnimLoader_.get()) {
                fallbackAnimLoader_ = animLoader_[index];
            }
        } else {
            LOGARG_INFO(LOGTYPE_DATA, "Unable to open anim%c from idx=%s, mul=%s", indexChar, idxPath.string().c_str(), path.string().c_str());
        }
    }

    free(animConfigEnabled);
    free(animConfigIdxPath);
    free(animConfigMulPath);
    free(animConfigHighDetailCount);
    free(animConfigLowDetailCount);
}

Manager::~Manager() {
    LOG_INFO(LOGTYPE_DATA, "Manager shutdown");
}

boost::filesystem::path Manager::getPathFor(Config& config, const char* configValue) {
    static boost::filesystem::path mulDirPath = config["/uome/files/mul-directory@path"].asPath();
    if (!config.exists(configValue)) {
        LOGARG_ERROR(LOGTYPE_DATA, "Trying to read unknown config vaule %s", configValue);
        throw Exception("Unknown config key: ", configValue);
    }

    boost::filesystem::path path = config[configValue].asPath();
    if (!boost::filesystem::exists(path)) {
        path = mulDirPath / path;
    }

    return path;
}

MapLoader* Manager::getMapLoader(unsigned int index) {
    if (index > 4) {
        LOGARG_WARN(LOGTYPE_DATA, "Trying to access map loader for map index %u", index);
        index = 0;
    }

    Manager* sing = getSingleton();

    MapLoader* ret = sing->mapLoader_[index].get();
    if (ret) {
        return ret;
    } else {
        LOGARG_WARN(LOGTYPE_DATA, "Trying to access uninitialized map index %u", index);
        return sing->fallbackMapLoader_.get();
    }
}

StaticsLoader* Manager::getStaticsLoader(unsigned int index) {
    if (index > 4) {
        LOGARG_WARN(LOGTYPE_DATA, "Trying to access statics loader for map index %u", index);
        index = 0;
    }

    Manager* sing = getSingleton();

    StaticsLoader* ret = sing->staticsLoader_[index].get();
    if (ret) {
        return ret;
    } else {
        LOGARG_WARN(LOGTYPE_DATA, "Trying to access uninitialized statics index %u", index);
        return sing->fallbackStaticsLoader_.get();
    }
}

boost::shared_ptr<ui::TextureProvider> Manager::getItemTextureProvider(unsigned int artId) {
    boost::shared_ptr<ui::TextureProvider> ret;

    // check for animate flag in tiledata
    if (getTileDataLoader()->getStaticTileInfo(artId)->animation()) {
        // if exists, load complex textureprovider
        ret.reset(new ui::AnimDataTextureProvider(artId));
    } else {
        // if not, just load the simple one
        ret.reset(new ui::SingleTextureProvider(artId));
    }

    return ret;
}

std::vector<boost::shared_ptr<ui::Animation> > Manager::getFullAnim(unsigned int animId) {
    // check .def files for correct anim file
    // load anim

    std::vector<boost::shared_ptr<ui::Animation> > ret;
    ret.push_back(getSingleton()->animLoader_[0]->getAnimation(animId));
    return ret;
}

AnimLoader* Manager::getAnimLoader(unsigned int index) {
    if (index > 5) {
        LOGARG_WARN(LOGTYPE_DATA, "Trying to access anim loader with index %u", index);
        index = 0;
    }

    Manager* sing = getSingleton();

    AnimLoader* ret = sing->animLoader_[index].get();
    if (ret) {
        return ret;
    } else {
        LOGARG_WARN(LOGTYPE_DATA, "Trying to access uninitialized anim loader index %u", index);
        return sing->fallbackAnimLoader_.get();
    }
}


}
}
