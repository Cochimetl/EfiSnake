#include <efi.h>
#include <efilib.h>
#include "util.h"
#define GAME_WIDTH 40
#define GAME_HEIGHT 20


UINTN gBorderColour = EFI_BACKGROUND_LIGHTGRAY;
UINTN gFruitColour = EFI_BACKGROUND_MAGENTA;


typedef enum
{
  UP,
  RIGHT,
  DOWN,
  LEFT
} Direction;


typedef struct
{
  UINTN x;
  UINTN y;
} GameNodePos;

typedef struct
{
  GameNodePos tail;
  GameNodePos head;
  Direction direction;
  UINTN length;
  UINTN colour;
} Snake;

Snake gSnake1;

Snake gSnake2;

typedef struct
{
  Snake *snake;
  Direction snakeDirection;
  BOOLEAN fruit;
} GameNode;

struct GameField
{
  GameNode field[GAME_WIDTH * GAME_HEIGHT];
  UINTN rowOffset;
  UINTN columnOffset;
} gField = {{}, 0, 0};



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
  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, node->snake ? node->snake->colour : node->fruit ? gFruitColour : EFI_BLACK);
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, L"  ");
  return EFI_SUCCESS;
}

EFI_STATUS snake_putFruit(EFI_SYSTEM_TABLE *SystemTable)
{
  UINTN randomValue = 0;
  util_rng(SystemTable, &randomValue);
  randomValue = randomValue % ((GAME_WIDTH * GAME_HEIGHT) - gSnake1.length - gSnake2.length);
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

EFI_STATUS snake_setupSnake(EFI_SYSTEM_TABLE *SystemTable, Snake *snake, GameNodePos head, GameNodePos tail, UINTN colour)
{
  snake->head = head;
  snake->tail = tail;
  snake->colour = colour;
  GameNodePos segmentPos = tail;
  BOOLEAN preferHorizontal = TRUE;
  Direction segmentDirection;
  while(TRUE)
  {
    GameNode * segment = snake_getGameNode(segmentPos);
    segment->snake = snake;
    snake_drawGameNode(SystemTable, segmentPos);

    BOOLEAN horizontalNeeded = segmentPos.x != head.x;
    BOOLEAN verticalNeeded = segmentPos.y != head.y;
    if((horizontalNeeded && verticalNeeded && preferHorizontal) ||
       (horizontalNeeded && !verticalNeeded))
    {
      segmentDirection = segmentPos.x < head.x ? RIGHT : LEFT;

      segmentPos.x += (segmentDirection == RIGHT ? 1 : -1);
      segment->snakeDirection = segmentDirection;
      snake->length++;
      preferHorizontal = FALSE;
    }
    else if((horizontalNeeded && verticalNeeded && !preferHorizontal) ||
       (!horizontalNeeded && verticalNeeded))
    {
      segmentDirection = segmentPos.y < head.y ? DOWN : UP;

      segmentPos.y += (segmentDirection == DOWN ? 1 : -1);
      segment->snakeDirection = segmentDirection;
      snake->length++;
      preferHorizontal = TRUE;
    }
    else
    {
        snake->direction = segmentDirection;
        return EFI_SUCCESS;
    }
  }
}

EFI_STATUS snake_setupField(EFI_SYSTEM_TABLE *SystemTable, BOOLEAN multiplayer)
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

  GameNodePos snake1Head = {4,1};
  GameNodePos snake1Tail = {1,1};
  snake_setupSnake(SystemTable, &gSnake1, snake1Head, snake1Tail, EFI_BACKGROUND_GREEN );

  if(multiplayer)
  {
    GameNodePos snake2Head = { GAME_WIDTH - 1 - 4, GAME_HEIGHT - 1 - 1};
    GameNodePos snake2Tail = { GAME_WIDTH - 1 - 1, GAME_HEIGHT - 1 - 1 };
    snake_setupSnake(SystemTable, &gSnake2, snake2Head, snake2Tail, EFI_BACKGROUND_RED );
  }
  gScore.total = 0;
  snake_drawScore(SystemTable);

  snake_putFruit(SystemTable);
  return EFI_SUCCESS;
}

EFI_STATUS snake_drawVictoryMessage(EFI_SYSTEM_TABLE *SystemTable, CHAR16 *message, UINTN colour)
{
  UINTN rows;
  UINTN columns;
  uefi_call_wrapper(SystemTable->ConOut->QueryMode, 4, SystemTable->ConOut, SystemTable->ConOut->Mode->Mode, &columns, &rows);

  UINTN offsetX = (columns - StrLen(message)) / 2;
  UINTN offsetY = rows / 2;

  uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, colour);
  uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, offsetX, offsetY);
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, message);

  return EFI_SUCCESS;
}

EFI_STATUS snake_moveSnake(EFI_SYSTEM_TABLE *SystemTable, Snake *snake)
{
  gScore.traveledDistance++;
  GameNodePos oldTailPos = snake->tail;
  GameNodePos oldHeadPos = snake->head;
  GameNode * oldTail = snake_getGameNode(oldTailPos);
  GameNode * oldHead = snake_getGameNode(oldHeadPos);
  Direction oldTailDirection = oldTail->snakeDirection;

  GameNodePos newTailPos = { oldTailPos.x + (oldTailDirection == RIGHT ? 1 : oldTailDirection == LEFT ? -1 : 0), 
                             oldTailPos.y + (oldTailDirection == DOWN ? 1 : oldTailDirection == UP ? -1 : 0)};
  GameNodePos newHeadPos = { oldHeadPos.x + (snake->direction == RIGHT ? 1 : snake->direction == LEFT ? -1 : 0), 
                             oldHeadPos.y + (snake->direction == DOWN ? 1 : snake->direction == UP ? -1 : 0)};

  if(newHeadPos.x >= GAME_WIDTH || newHeadPos.y >= GAME_HEIGHT)
  {
    return EFI_NOT_FOUND;
  }

  oldHead->snakeDirection = snake->direction;
  GameNode *newHead = snake_getGameNode(newHeadPos);
  if(newHead->snake)
  {
   return EFI_NOT_FOUND;
  }


  newHead->snake = snake;
  snake_drawGameNode(SystemTable, newHeadPos);
  snake->head = newHeadPos;


  if(newHead->fruit)
  {
    newHead->fruit = FALSE;
    snake->length++;
    gScore.total += snake_scorePoints();
    snake_drawScore(SystemTable);
    snake_putFruit(SystemTable);
  }
  else
  {
    oldTail->snake = NULL;
    snake_drawGameNode(SystemTable, oldTailPos);
    snake->tail = newTailPos;
  }


  return EFI_SUCCESS;
}

EFI_STATUS snake_singleplayer(EFI_SYSTEM_TABLE *SystemTable, UINTN * score)
{

  const EFI_INPUT_KEY KEY_UP = {1, 0};
  const EFI_INPUT_KEY KEY_W = {0, 119};

  const EFI_INPUT_KEY KEY_DOWN = {2, 0};
  const EFI_INPUT_KEY KEY_S = {0, 115};

  const EFI_INPUT_KEY KEY_RIGHT = {3, 0};
  const EFI_INPUT_KEY KEY_D = {0, 100};

  const EFI_INPUT_KEY KEY_LEFT = {4, 0};
  const EFI_INPUT_KEY KEY_A = {0, 97};



  snake_setupField(SystemTable, FALSE);


  uefi_call_wrapper(SystemTable->ConIn->Reset, 2, SystemTable->ConIn, FALSE);
  while(uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 1 * 100 * 1000) == EFI_SUCCESS)
  {
    EFI_INPUT_KEY key;
    Direction oldDirection = gSnake1.direction;
    while(uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &key) == EFI_SUCCESS)
    {
      Direction newDirection;
      if(util_keysEqual(key, KEY_UP) || util_keysEqual(key, KEY_W))
      {
        newDirection = UP;
      }
      else if(util_keysEqual(key, KEY_DOWN) || util_keysEqual(key, KEY_S))
      {
        newDirection = DOWN;
      }
      else if(util_keysEqual(key, KEY_RIGHT) || util_keysEqual(key, KEY_D))
      {
        newDirection = RIGHT;
      }
      else if(util_keysEqual(key, KEY_LEFT) || util_keysEqual(key, KEY_A))
      {
        newDirection = LEFT;
      }
      else
      {
        continue;
      }

      INTN directionDifference = newDirection > oldDirection ? newDirection - oldDirection : oldDirection - newDirection;
      if(directionDifference != 2) gSnake1.direction = newDirection;
    }

    if(snake_moveSnake(SystemTable, &gSnake1) != EFI_SUCCESS)
    {
     *score = gScore.total;
     return EFI_SUCCESS;
    }
  }
  return EFI_DEVICE_ERROR;
}

EFI_STATUS snake_multiplayer(EFI_SYSTEM_TABLE *SystemTable, UINTN * score)
{

  const EFI_INPUT_KEY KEY_UP = {1, 0};
  const EFI_INPUT_KEY KEY_W = {0, 119};

  const EFI_INPUT_KEY KEY_DOWN = {2, 0};
  const EFI_INPUT_KEY KEY_S = {0, 115};

  const EFI_INPUT_KEY KEY_RIGHT = {3, 0};
  const EFI_INPUT_KEY KEY_D = {0, 100};

  const EFI_INPUT_KEY KEY_LEFT = {4, 0};
  const EFI_INPUT_KEY KEY_A = {0, 97};



  snake_setupField(SystemTable, TRUE);


  uefi_call_wrapper(SystemTable->ConIn->Reset, 2, SystemTable->ConIn, FALSE);
  while(uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 1 * 100 * 1000) == EFI_SUCCESS)
  {
    EFI_INPUT_KEY key;
    Direction oldDirection1 = gSnake1.direction;
    Direction oldDirection2 = gSnake2.direction;
    while(uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &key) == EFI_SUCCESS)
    {
      Direction newDirection;
      if(util_keysEqual(key, KEY_UP) || util_keysEqual(key, KEY_W))
      {
        newDirection = UP;
      }
      else if(util_keysEqual(key, KEY_DOWN) || util_keysEqual(key, KEY_S))
      {
        newDirection = DOWN;
      }
      else if(util_keysEqual(key, KEY_RIGHT) || util_keysEqual(key, KEY_D))
      {
        newDirection = RIGHT;
      }
      else if(util_keysEqual(key, KEY_LEFT) || util_keysEqual(key, KEY_A))
      {
        newDirection = LEFT;
      }
      else
      {
        continue;
      }


      Snake *snake;
      Direction oldDirection;
      if(util_keysEqual(key, KEY_UP) || util_keysEqual(key, KEY_RIGHT) || util_keysEqual(key, KEY_DOWN) || util_keysEqual(key, KEY_LEFT))
      {
        snake = &gSnake1;
        oldDirection = oldDirection1;
      }
      else if(util_keysEqual(key, KEY_W) || util_keysEqual(key, KEY_D) || util_keysEqual(key, KEY_S) || util_keysEqual(key, KEY_A))
      {
        snake = &gSnake2;
        oldDirection = oldDirection2;
      }


      INTN directionDifference = newDirection > oldDirection ? newDirection - oldDirection : oldDirection - newDirection;
      if(directionDifference != 2) snake->direction = newDirection;
    }

    EFI_STATUS snake1Result = snake_moveSnake(SystemTable, &gSnake1);
    EFI_STATUS snake2Result = snake_moveSnake(SystemTable, &gSnake2);

    if(snake1Result != EFI_SUCCESS || snake2Result != EFI_SUCCESS)
    {
      CHAR16* message = snake1Result == EFI_SUCCESS ? L"Player 1 wins!" : snake2Result == EFI_SUCCESS ? L"Player 2 wins!" : L"Draw!";
      UINTN messageColour = snake1Result == EFI_SUCCESS ? gSnake1.colour : snake2Result == EFI_SUCCESS ? gSnake2.colour : EFI_YELLOW;
      snake_drawVictoryMessage(SystemTable, message, messageColour);
      uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 3 * 1000 * 1000);
      *score = gScore.total;
      return EFI_SUCCESS;
    }
  }
  return EFI_DEVICE_ERROR;
}
