#include "field.h"

#include "../util/client_error.h"
#include "../util/string.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <queue>
#include <random>

size_t ToIndexUnsafe(u8 width, u8 x, u8 y) {
    return size_t(y) * width + x;
}

bool VerifyDimension(u8 dimension) {
    return dimension < 9 || dimension > 30;
}

u8 VerifyWidth(u8 width) {
    if (!VerifyDimension(width)) {
        throw ClientError("Width should be between 9 and 30");
    }
    return width;
}

u8 VerifyHeight(u8 height) {
    if (!VerifyDimension(height)) {
        throw ClientError("Height should be between 9 and 30");
    }
    return height;
}

u32 VerifyMineCount(u32 area, u32 mineCount) {
    if (mineCount == 0 || mineCount + 1 >= area) {
        throw ClientError(String{"Wrong amount of mines: "} + std::to_string(mineCount));
    }
    return mineCount;
}

Field::Field(u8 width, u8 height, u32 mineCount, u32 seed)
    : Width(VerifyWidth(width))
    , Height(VerifyHeight(height))
    , MineCount(VerifyMineCount(u32(Width) * Height, mineCount))
    , Seed(seed)
    , Cells(u32(Width) * Height)
    , IsUntouched(true)
{
}

Field::OpenCellResult Field::OpenCell(u8 x, u8 y) {
    auto& cell = Get(x, y);
    if (IsUntouched) {
        GenerateMines(x, y);
        IsUntouched = false;
    }

    if (cell.HasMine) {
        return {Field::ActionType::EXPLODE};
    }

    if (cell.IsOpen) {
        return {Field::ActionType::CELL_IS_ALREADY_OPEN};
    }

    if (cell.HasFlag) {
        return {Field::ActionType::CELL_HAS_FLAG};
    }

    return {Field::ActionType::NEW_CELLS_OPEN, OpenNewCells(x, y)};
}

Field::PlaceFlagResult Field::PlaceFlag(u8 x, u8 y) {
    auto& cell = Get(x, y);
    if (cell.IsOpen) {
        return {Field::ActionType::CELL_IS_ALREADY_OPEN};
    }
    
    cell.HasFlag = !cell.HasFlag;
    const bool flagPlaced = cell.HasFlag;
    return {flagPlaced
            ? Field::ActionType::FLAG_PLACED
            : Field::ActionType::FLAG_REMOVED};
}

size_t Field::ToIndex(u8 x, u8 y) const {
    size_t idx = size_t(y) * Width + x;
    if (idx >= Cells.size()) {
        throw ClientError("Wrong cell indices: "
                          + std::to_string(x) + ", "
                          + std::to_string(y));
    }
    return idx;
}

Field::Cell& Field::Get(u8 x, u8 y) {
    return Cells[ToIndex(x, y)];
}

Field::Cell& Field::GetUnsafe(u8 x, u8 y) {
    return Cells[ToIndexUnsafe(Width, x, y)];
}

std::vector<Field::NewOpenCell> Field::OpenNewCells(u8 x, u8 y) {
    // Let's do some BFS!
    std::vector<Field::NewOpenCell> result;
    std::queue<Field::NewOpenCell> toOpen;

    toOpen.push({x, y});
    while (!toOpen.empty()) {
        const auto& coords = toOpen.front();
        auto& cell = GetUnsafe(coords.X, coords.Y);
        result.push_back(coords);
        cell.IsOpen = true;

        // BEWARE: CRAPCODE!
        if (coords.X > 0) {
            auto& candidate = GetUnsafe(coords.X, coords.Y);
            if (!candidate.IsOpen && !candidate.HasMine) {
                toOpen.push({u8(coords.X - 1), coords.Y});
            }
        }
        if (coords.X + 1 < Width) {
            auto& candidate = GetUnsafe(coords.X, coords.Y);
            if (!candidate.IsOpen && !candidate.HasMine) {
                toOpen.push({u8(coords.X + 1), coords.Y});
            }
        }
        if (coords.Y > 0) {
            auto& candidate = GetUnsafe(coords.X, coords.Y);
            if (!candidate.IsOpen && !candidate.HasMine) {
                toOpen.push({coords.X, u8(coords.Y - 1)});
            }
        }
        if (coords.Y + 1 < Height) {
            auto& candidate = GetUnsafe(coords.X, coords.Y);
            if (!candidate.IsOpen && !candidate.HasMine) {
                toOpen.push({coords.X, u8(coords.Y + 1)});
            }
        }

        toOpen.pop();
    }

    return result;
}

void Field::GenerateMines(u8 x, u8 y) {
    const size_t origin = ToIndex(x, y);
    const size_t area = size_t(x) * y;
    std::vector<size_t> possibleIndices(area);

    std::iota(possibleIndices.begin(), possibleIndices.end(), 0);
    std::swap(possibleIndices[origin], possibleIndices.back());
    possibleIndices.resize(possibleIndices.size() - 1);

    std::vector<size_t> mineIndices;
    mineIndices.reserve(MineCount);
    std::sample(possibleIndices.begin(), possibleIndices.end(), std::back_inserter(mineIndices),
                MineCount, std::mt19937(Seed));

    for (const auto idx : mineIndices) {
        Cells[idx].HasMine = true;
    }
}
