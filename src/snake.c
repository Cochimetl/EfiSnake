#include <efi.h>
#include <efilib.h>
#include "util.h"
#define GAME_WIDTH 40
#define GAME_HEIGHT 20


UINTN gBorderColour = EFI_BACKGROUND_LIGHTGRAY;
UINTN gFruitColour = EFI_BACKGROUND_MAGENTA;
UINTN gSnakeColour = EFI_BACKGROUND_GREEN;


typedef enum
{
  UP,
  DOWN,
  LEFT,
  RIGHT
} Direction;

typedef struct
{
  BOOLEAN snake;
  Direction snakeDirection;
  BOOLEAN fruit;
} GameNode;

struct GameField
{
  GameNode field[GAME_WIDTH * GAME_HEIGHT];
  UINTN rowOffset;
  UINTN columnOffset;
} gField = {{}, 0, 0};


typedef struct
{
  UINTN x;
  UINTN y;
} GameNodePos;

struct Snake
{
  GameNodePos tail;
  GameNodePos head;
  Direction direction;
  UINTN length;
} gSnake;

struct Score
{
  UINTN total;
  UINTN traveledDistance;
} gScore;

UINTN snake_scorePoints()
{
  if(gScore.traveledDistance <= 5)
  {
    return 100;
  }
  else if(gScore.traveledDistance <= 10)
  {
    return 50;
  }
  else if(gScore.traveledDistance < 25)
  {
    return 25;
  }
  else if(gScore.traveledDistance < 40)
  {
    return 10;
  }
  else
  {
    return 5;
  }
}

EFI_STATUS snake_drawScore(EFI_SYSTEM_TABLE *SystemTable)
{
  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, EFI_WHITE);
  uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, gField.columnOffset + ((GAME_WIDTH * 2) * 0.9), gField.rowOffset + GAME_HEIGHT + 3);
  Print(L"%d", gScore.total);
  return EFI_SUCCESS;
}

GameNode * snake_getGameNode(GameNodePos pos)
{
  if(pos.x >= GAME_WIDTH || pos.y >= GAME_HEIGHT) return NULL;

  return &gField.field[pos.x * GAME_HEIGHT + pos.y];
}

EFI_STATUS snake_drawGameNode(EFI_SYSTEM_TABLE *SystemTable, GameNodePos pos)
{
  GameNode *node = snake_getGameNode(pos);
  uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, gField.columnOffset + pos.x * 2, gField.rowOffset + pos.y);
  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, node->snake ? gSnakeColour : node->fruit ? gFruitColour : EFI_BLACK);
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, L"  ");
  return EFI_SUCCESS;
}

EFI_STATUS snake_putFruit(EFI_SYSTEM_TABLE *SystemTable)
{
  UINTN randomValue = 0;
  util_rng(SystemTable, &randomValue);
  randomValue = randomValue % ((GAME_WIDTH * GAME_HEIGHT) - gSnake.length);
  for(UINTN x = 0; x < GAME_WIDTH; x++)
  {
    for(UINTN y = 0; y < GAME_HEIGHT; y++)
    {
      GameNodePos nodePos = { x, y };
      GameNode *node = snake_getGameNode(nodePos);
      if(node->snake) continue;
      if(randomValue-- == 0)
      {
        node->fruit = TRUE;
        snake_drawGameNode(SystemTable, nodePos);
        gScore.traveledDistance = 0;
        return EFI_SUCCESS;
      }
    }
  }
  return EFI_NOT_FOUND;
}


EFI_STATUS snake_setupField(EFI_SYSTEM_TABLE *SystemTable)
{
  uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
  SetMem(&gField.field[0], sizeof(GameNode) * GAME_WIDTH * GAME_HEIGHT, 0);


  UINTN rows;
  UINTN columns;
  uefi_call_wrapper(SystemTable->ConOut->QueryMode, 4, SystemTable->ConOut, SystemTable->ConOut->Mode->Mode, &columns, &rows);

  gField.rowOffset = (rows - GAME_HEIGHT) / 2;
  gField.columnOffset = (columns - GAME_WIDTH * 2) / 2;
  UINTN borderRowOffset = gField.rowOffset - 1;
  UINTN borderColumnOffset = gField.columnOffset - 2;
  UINTN borderWidth = GAME_WIDTH * 2 + 4;
  UINTN borderHeight = GAME_HEIGHT + 2;

  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, gBorderColour);
  for(UINTN row = 0; row < borderHeight; row++)
  {
      uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, borderColumnOffset, borderRowOffset + row);
      uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, L"  ");
      uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, borderColumnOffset + borderWidth - 2, borderRowOffset + row);
      uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, L"  ");
  }
  for(UINTN column = 0; column < borderWidth; column++)
  {
      uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, borderColumnOffset + column, borderRowOffset);
      uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, L" ");
      uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, borderColumnOffset + column, borderRowOffset + borderHeight - 1);
      uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, L" ");
  }

  gSnake.head.x = 5;
  gSnake.head.y = 5;
  GameNode * head = snake_getGameNode(gSnake.head);
  head->snake = TRUE;
  head->snakeDirection = RIGHT;
  snake_drawGameNode(SystemTable, gSnake.head);
  gSnake.tail.x = 3;
  gSnake.tail.y = 5;
  GameNode * tail = snake_getGameNode(gSnake.tail);
  tail->snake = TRUE;
  tail->snakeDirection = RIGHT;
  snake_drawGameNode(SystemTable, gSnake.tail);

  GameNodePos middlePos = { 4, 5 };
  GameNode * middle = snake_getGameNode(middlePos);
  middle->snake = TRUE;
  middle->snakeDirection = RIGHT;
  snake_drawGameNode(SystemTable, middlePos);

  gSnake.direction = RIGHT;
  gSnake.length = 3;

  gScore.total = 0;
  snake_drawScore(SystemTable);

  snake_putFruit(SystemTable);
  return EFI_SUCCESS;
}

EFI_STATUS snake_moveSnake(EFI_SYSTEM_TABLE *SystemTable)
{
  gScore.traveledDistance++;
  GameNodePos oldTailPos = gSnake.tail;
  GameNodePos oldHeadPos = gSnake.head;
  GameNode * oldTail = snake_getGameNode(oldTailPos);
  GameNode * oldHead = snake_getGameNode(oldHeadPos);
  Direction oldTailDirection = oldTail->snakeDirection;

  GameNodePos newTailPos = { oldTailPos.x + (oldTailDirection == RIGHT ? 1 : oldTailDirection == LEFT ? -1 : 0), 
                             oldTailPos.y + (oldTailDirection == DOWN ? 1 : oldTailDirection == UP ? -1 : 0)};
  GameNodePos newHeadPos = { oldHeadPos.x + (gSnake.direction == RIGHT ? 1 : gSnake.direction == LEFT ? -1 : 0), 
                             oldHeadPos.y + (gSnake.direction == DOWN ? 1 : gSnake.direction == UP ? -1 : 0)};

  if(newHeadPos.x >= GAME_WIDTH || newHeadPos.y >= GAME_HEIGHT)
  {
    return EFI_NOT_FOUND;
  }

  oldHead->snakeDirection = gSnake.direction;
  GameNode *newHead = snake_getGameNode(newHeadPos);
  if(newHead->snake)
  {
   return EFI_NOT_FOUND;
  }


  newHead->snake = TRUE;
  snake_drawGameNode(SystemTable, newHeadPos);
  gSnake.head = newHeadPos;


  if(newHead->fruit)
  {
    newHead->fruit = FALSE;
    gSnake.length = gSnake.length + 1;
    gScore.total += snake_scorePoints();
    snake_drawScore(SystemTable);
    snake_putFruit(SystemTable);
  }
  else
  {
    oldTail->snake = FALSE;
    snake_drawGameNode(SystemTable, oldTailPos);
    gSnake.tail = newTailPos;
  }


  return EFI_SUCCESS;
}

EFI_STATUS snake_passControl(EFI_SYSTEM_TABLE *SystemTable, UINTN * score)
{

  const EFI_INPUT_KEY KEY_UP = {1, 0};
  const EFI_INPUT_KEY KEY_W = {0, 119};

  const EFI_INPUT_KEY KEY_DOWN = {2, 0};
  const EFI_INPUT_KEY KEY_S = {0, 115};

  const EFI_INPUT_KEY KEY_RIGHT = {3, 0};
  const EFI_INPUT_KEY KEY_D = {0, 100};

  const EFI_INPUT_KEY KEY_LEFT = {4, 0};
  const EFI_INPUT_KEY KEY_A = {0, 97};



  snake_setupField(SystemTable);


  uefi_call_wrapper(SystemTable->ConIn->Reset, 2, SystemTable->ConIn, FALSE);
  while(uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 1 * 100 * 1000) == EFI_SUCCESS)
  {
    EFI_INPUT_KEY key;
    while(uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &key) == EFI_SUCCESS)
    {
      if(util_keysEqual(key, KEY_UP) || util_keysEqual(key, KEY_W))
      {
        gSnake.direction = UP;
      }
      else if(util_keysEqual(key, KEY_DOWN) || util_keysEqual(key, KEY_S))
      {
        gSnake.direction = DOWN;
      }
      else if(util_keysEqual(key, KEY_RIGHT) || util_keysEqual(key, KEY_D))
      {
        gSnake.direction = RIGHT;
      }
      else if(util_keysEqual(key, KEY_LEFT) || util_keysEqual(key, KEY_A))
      {
        gSnake.direction = LEFT;
      }
    }

    if(snake_moveSnake(SystemTable) != EFI_SUCCESS)
    {
     return EFI_SUCCESS;
    }
  }
  return EFI_DEVICE_ERROR;
}
