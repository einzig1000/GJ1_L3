#include "CollisionTag.h"
#include"Utilities/Json/JsonManager.h"
#include"Utilities/Logger/Logger.h"

uint32_t CollisionTag::useIndex_ = 0;
const uint32_t CollisionTag::maxCount_ = 1000;

std::unordered_map<std::string, uint32_t>CollisionTag::tags_;
std::unordered_map< uint32_t, std::string>CollisionTag::names_;

namespace {
    std::string filePath = "assets/application/json/Collision/TagName.json";
}

void CollisionTag::AddTag(const std::string& tagName)
{
    if (tags_.contains(tagName) || useIndex_ >= maxCount_) {
        return;
    };

    useIndex_++;

    tags_[tagName] = 0b1 << useIndex_;
    names_[tags_[tagName]] = tagName;

}

uint32_t CollisionTag::GetTag(const std::string& tagName)
{
    if (tags_.contains(tagName)) {
        return tags_.at(tagName);
    };

    Log("This tagName is no attributes");

    //何にも属さないを 返す
    return 0;
}

const std::string CollisionTag::GetTagName(uint32_t tagNum)
{
    if (names_.contains(tagNum)) {
        return names_.at(tagNum);
    }

    Log("This tagName is unknown");

    return "unKnown";
}

void CollisionTag::SaveTagNames()
{
  /*  JsonManager::Save(filePath);*/
}

void CollisionTag::LoadTagNames()
{

   //JsonManager::Load(filePath);

   // for (auto& data : JsonManager::GetJsonData(filePath)) {
   //     //タグを追加する
   //     AddTag(data);
   // }

}