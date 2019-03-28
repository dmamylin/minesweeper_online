#pragma once

#include "../types.h"

#include <vector>

class Field {
public:
    enum class ActionType : u8 {
        NEW_CELLS_OPEN,
        EXPLODE,
        CELL_IS_ALREADY_OPEN,
        CELL_HAS_FLAG,
        FLAG_PLACED,
        FLAG_REMOVED
    };

    struct NewOpenCell {
        const u8 X = 0;
        const u8 Y = 0;
    };

    struct OpenCellResult {
        const ActionType Type;
        const std::vector<NewOpenCell> NewOpenCells = {};
    };

    struct PlaceFlagResult {
        const ActionType Type = ActionType::CELL_HAS_FLAG;
    };

public:
    Field(u8 width, u8 height, u32 mineCount, u32 seed);

    OpenCellResult OpenCell(u8 x, u8 y);
    PlaceFlagResult PlaceFlag(u8 x, u8 y);

public:
    Field(const Field&) = delete;
    Field& operator=(const Field&) = delete;

private:
    struct Cell {
        bool HasMine = false;
        bool IsOpen = false;
        bool HasFlag = false;
    };

private:
    u8 Width;
    u8 Height;
    u32 MineCount;
    u32 Seed;
    std::vector<Cell> Cells;
    bool IsUntouched;

private:
    size_t ToIndex(u8 x, u8 y) const;
    Cell& Get(u8 x, u8 y);
    Cell& GetUnsafe(u8 x, u8 y);
    std::vector<NewOpenCell> OpenNewCells(u8 x, u8 y);
    void GenerateMines(u8 x, u8 y);
};
