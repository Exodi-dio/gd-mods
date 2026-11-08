#pragma once

#include <Geode/loader/Dispatch.hpp>
#define MY_MOD_ID "cvolton.level-id-api"

#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJLevelList.hpp>

namespace EditorIDs {
    /**
     * @brief Get the editor ID of a level or list.
     *
     * @param level The level to get the ID of.
     *
     * @return The ID of the level, or m_levelID if the level is not editor.
     */
    int getID(GJGameLevel* level);

    /**
     * @brief Get the editor ID of a level or list.
     *
     * @param list The list to get the ID of.
     *
     * @return The ID of the level, or m_levelID if the level is not editor.
     */
    int getID(GJLevelList* list);

    /**
     * @brief Get the editor ID of a level or list.
     *
     * @param level The level to get the ID of.
     * @param autoAssign Whether to assign an ID if it has not been assigned automatically
     *
     * @return The ID of the level, or m_levelID if the level is not editor.
     */
    int getID(GJGameLevel* level, bool autoAssign);

    /**
     * @brief Get the editor ID of a level or list.
     *
     * @param list The list to get the ID of.
     * @param autoAssign Whether to assign an ID if it has not been assigned automatically
     *
     * @return The ID of the level, or m_levelID if the level is not editor.
     */
    int getID(GJLevelList* list, bool autoAssign);

    /**
     * @brief Get a level by its editor ID.
     * 
     * @param id The editor ID of the level.
     *
     * @return The level with the given ID, or nullptr if no level has that ID.
     */
    GJGameLevel* getLevelByID(int id);

    /**
     * @brief Get a list by its editor ID.
     * 
     * @param id The editor ID of the list.
     *
     * @return The list with the given ID, or nullptr if no list has that ID.
     */
    GJLevelList* getListByID(int id);
}

//#undef MY_MOD_ID