#include "entity_factory.h"
#include "platform.h"
#include "moving_platform.h"
#include "crumble_tile.h"
#include "spikes.h"
#include "spinner.h"
#include "bounce_pad.h"
#include "checkpoint.h"
#include "finish_zone.h"
#include "button.h"
#include "door.h"
#include "walker.h"
#include "flyer.h"
#include "bump_block.h"
#include "constants.h"
#include "level_error.h"
#include <algorithm>

namespace obby {

std::unique_ptr<Entity> EntityFactory::createEntity(const std::string& type, std::istringstream& ss, int worldId) {
    if (type == "box") {
        float x, y, z, sx, sy, sz;
        int cIndex = 0;
        if (ss >> x >> y >> z >> sx >> sy >> sz) {
            if (!(ss >> cIndex)) cIndex = 0;
            return std::make_unique<Platform>(Vec3{ x, y, z }, Vec3{ sx, sy, sz }, cIndex, worldId);
        }
        throw LevelParseError("Invalid box entity parameters");
    } else if (type == "checkpoint") {
        float x, y, z;
        if (ss >> x >> y >> z) {
            return std::make_unique<Checkpoint>(Vec3{ x, y, z });
        }
        throw LevelParseError("Invalid checkpoint entity parameters");
    } else if (type == "finish") {
        float x, y, z, sx, sy, sz;
        if (ss >> x >> y >> z >> sx >> sy >> sz) {
            return std::make_unique<FinishZone>(Vec3{ x, y, z }, Vec3{ sx, sy, sz });
        }
        throw LevelParseError("Invalid finish entity parameters");
    } else if (type == "mover") {
        float x, y, z, sx, sy, sz, dx, dy, dz, period;
        int cIndex = 0;
        if (ss >> x >> y >> z >> sx >> sy >> sz >> dx >> dy >> dz >> period) {
            if (!(ss >> cIndex)) cIndex = 0;
            return std::make_unique<MovingPlatform>(Vec3{ x, y, z }, Vec3{ sx, sy, sz }, Vec3{ dx, dy, dz }, std::max(0.1f, period), cIndex, worldId);
        }
        throw LevelParseError("Invalid mover entity parameters");
    } else if (type == "spinner") {
        float x, y, z, length, height, speed;
        if (ss >> x >> y >> z >> length >> height >> speed) {
            return std::make_unique<Spinner>(Vec3{ x, y, z }, length, height, speed);
        }
        throw LevelParseError("Invalid spinner entity parameters");
    } else if (type == "crumble") {
        float x, y, z, sx, sy, sz, delay, respawnTime;
        if (ss >> x >> y >> z >> sx >> sy >> sz >> delay >> respawnTime) {
            return std::make_unique<CrumbleTile>(Vec3{ x, y, z }, Vec3{ sx, sy, sz }, std::max(0.1f, delay), std::max(0.1f, respawnTime), worldId);
        }
        throw LevelParseError("Invalid crumble entity parameters");
    } else if (type == "spikes") {
        float x, y, z, sx, sy, sz;
        if (ss >> x >> y >> z >> sx >> sy >> sz) {
            return std::make_unique<Spikes>(Vec3{ x, y, z }, Vec3{ sx, sy, sz });
        }
        throw LevelParseError("Invalid spikes entity parameters");
    } else if (type == "bounce") {
        float x, y, z, sx, sy, sz, strength;
        if (ss >> x >> y >> z >> sx >> sy >> sz >> strength) {
            return std::make_unique<BouncePad>(Vec3{ x, y, z }, Vec3{ sx, sy, sz }, strength);
        }
        throw LevelParseError("Invalid bounce entity parameters");
    } else if (type == "button") {
        int bId = 0;
        float x, y, z, dur;
        if (ss >> bId >> x >> y >> z >> dur) {
            return std::make_unique<Button>(bId, Vec3{ x, y, z }, std::max(0.0f, dur));
        }
        throw LevelParseError("Invalid button entity parameters");
    } else if (type == "door") {
        int dId = 0;
        float x, y, z, sx, sy, sz;
        if (ss >> dId >> x >> y >> z >> sx >> sy >> sz) {
            return std::make_unique<Door>(dId, Vec3{ x, y, z }, Vec3{ sx, sy, sz });
        }
        throw LevelParseError("Invalid door entity parameters");
    } else if (type == "walker") {
        std::vector<float> args;
        float val;
        while (ss >> val) args.push_back(val);
        if (args.size() == 2) {
            return std::make_unique<Walker>(Vec3{ args[0], args[1], 0.0f }, 0.0f);
        } else if (args.size() == 3) {
            return std::make_unique<Walker>(Vec3{ args[0], args[1], 0.0f }, args[2]);
        } else if (args.size() == 4) {
            return std::make_unique<Walker>(Vec3{ args[0], args[1], args[2] }, args[3]);
        } else if (args.size() >= 5) {
            return std::make_unique<Walker>(Vec3{ args[0], args[1], args[2] }, args[3], args[4]);
        }
        throw LevelParseError("Invalid walker entity parameters");
    } else if (type == "flyer") {
        std::vector<float> args;
        float val;
        while (ss >> val) args.push_back(val);
        if (args.size() == 4) {
            return std::make_unique<Flyer>(Vec3{ args[0], args[1], 0.0f }, args[2], args[3]);
        } else if (args.size() == 5) {
            return std::make_unique<Flyer>(Vec3{ args[0], args[1], args[2] }, args[3], args[4]);
        } else if (args.size() >= 6) {
            return std::make_unique<Flyer>(Vec3{ args[0], args[1], args[2] }, args[3], args[4], Constants::FLYER_SPEED, args[5]);
        }
        throw LevelParseError("Invalid flyer entity parameters");
    } else if (type == "bump") {
        std::vector<float> args;
        float val;
        while (ss >> val) args.push_back(val);
        if (args.size() == 2) {
            return std::make_unique<BumpBlock>(Vec3{ args[0], args[1], 0.0f }, 1, worldId);
        } else if (args.size() == 3) {
            return std::make_unique<BumpBlock>(Vec3{ args[0], args[1], 0.0f }, static_cast<int>(args[2]), worldId);
        } else if (args.size() == 4) {
            return std::make_unique<BumpBlock>(Vec3{ args[0], args[1], args[2] }, static_cast<int>(args[3]), worldId);
        } else if (args.size() >= 5) {
            return std::make_unique<BumpBlock>(Vec3{ args[0], args[1], args[2] }, static_cast<int>(args[3]), static_cast<int>(args[4]));
        }
        throw LevelParseError("Invalid bump entity parameters");
    }
    return nullptr;
}

} // namespace obby
