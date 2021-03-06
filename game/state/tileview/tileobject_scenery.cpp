#include "game/state/tileview/tileobject_scenery.h"
#include "framework/renderer.h"
#include "game/state/city/scenery.h"
#include "game/state/rules/scenery_tile_type.h"
#include "game/state/tileview/tile.h"

namespace OpenApoc
{

void TileObjectScenery::draw(Renderer &r, TileTransform &transform, Vec2<float> screenPosition,
                             TileViewMode mode, bool, int, bool, bool)
{
	std::ignore = transform;
	// Mode isn't used as TileView::tileToScreenCoords already transforms according to the mode
	auto scenery = this->scenery.lock();
	if (!scenery)
	{
		LogError("Called with no owning scenery object");
		return;
	}
	// FIXME: If damaged use damaged tile sprites?
	auto &type = scenery->type;
	sp<Image> sprite;
	sp<Image> overlaySprite;
	Vec2<float> transformedScreenPos = screenPosition;
	switch (mode)
	{
		case TileViewMode::Isometric:
			sprite = type->sprite;
			overlaySprite = type->overlaySprite;
			transformedScreenPos -= type->imageOffset;
			break;
		case TileViewMode::Strategy:
			sprite = type->strategySprite;
			// All strategy sprites so far are 8x8 so offset by 4 to draw from the center
			// FIXME: Not true for large sprites (2x2 UFOs?)
			transformedScreenPos -= Vec2<float>{4, 4};
			break;
		default:
			LogError("Unsupported view mode");
	}
	if (sprite)
		r.draw(sprite, transformedScreenPos);
	// FIXME: Should be drawn at 'later' Z than scenery (IE on top of any vehicles on tile?)
	if (overlaySprite)
		r.draw(overlaySprite, transformedScreenPos);
}

TileObjectScenery::~TileObjectScenery() = default;

TileObjectScenery::TileObjectScenery(TileMap &map, sp<Scenery> scenery)
    : TileObject(map, Type::Scenery, Vec3<float>{1, 1, 1}), scenery(scenery)
{
}

sp<Scenery> TileObjectScenery::getOwner() const
{
	auto s = this->scenery.lock();
	if (!s)
	{
		LogError("Owning scenery object disappeared");
	}
	return s;
}

sp<VoxelMap> TileObjectScenery::getVoxelMap(Vec3<int>, bool) const
{
	return this->getOwner()->type->voxelMap;
}

Vec3<float> TileObjectScenery::getPosition() const
{
	auto s = this->scenery.lock();
	if (!s)
	{
		LogError("Called with no owning scenery object");
		return {0, 0, 0};
	}
	return s->getPosition();
}

float TileObjectScenery::getZOrder() const
{
	// FIXME: Hack to force 'scenery' objects to be half-a-tile down in Z
	// The formula to calculate "3.5f" is: (tile_x + tile_y + tile_z) / tile_z /2
	return getCenter().z - 3.5f + (float)getType() / 1000.0f;
}

} // namespace OpenApoc
