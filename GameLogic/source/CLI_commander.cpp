#include <iostream>
#include <string>
#include <vector>

#include "Game_master.hpp"
#include "Commander.hpp"
#include "Description_generator.hpp"

#define TILE_WIDTH_CHARS	6
#define TILE_HEIGHT_CHARS	6

CLI_commander::CLI_commander() : desc_gen(Description_generator::get()) {}

void CLI_commander::render_UI()
{
   // GRID _________________________________________
     //draw the empty field
     std::string buffer = ".";

     for(int i = 0; i < grid_width; i++)
        buffer.append(std::string(TILE_WIDTH_CHARS, ' ').append("."));  //first divider line

     buffer.append("\n");

     for(int j = 0; j < grid_height; j++)
     {
        // empty spaces
        for(int i = 0; i < TILE_HEIGHT_CHARS; i++)
            buffer.append(std::string(grid_width*(TILE_WIDTH_CHARS+1), ' ').append(" \n"));  

        // divider
        buffer.append(".");

        for(int i = 0; i < grid_width; i++)
        buffer.append(std::string(TILE_WIDTH_CHARS, ' ').append("."));

        buffer.append("\n");
     }

     // Highlight deploy zones
     for(int i = 0; i < grid_height; i++)
     {
         highlight_tile_subtle(buffer, grid_width, i, 0);
         highlight_tile_subtle(buffer, grid_width, i, grid_width - 1);
     }

     // Render Units
   for(Card_info card : active_cards)
   {
      Description_generator::Card_descr original = desc_gen.get_card_instance(card.global_id);
      if(card.type == CTYPE_UNIT)
      {
         if(card.y == 3 || card.y == 4) //in capture zone
            highlight_tile_funky(buffer, grid_width, card.x, card.y);
         else
            highlight_tile_bold(buffer, grid_width, card.x, card.y);

         // Name
         for(int bias = 1; bias < (TILE_WIDTH_CHARS+1) && bias < original.name.size() + 1; bias++)
            buffer[(card.x*(TILE_HEIGHT_CHARS+1) + 1)*(grid_width*(TILE_WIDTH_CHARS+1) + 2) \
            	   + card.y*(TILE_WIDTH_CHARS+1) + bias] = original.name[bias-1];

         for(int bias = original.name.size() + 1; bias < (TILE_WIDTH_CHARS+1); bias++)
            buffer[(card.x*(TILE_HEIGHT_CHARS+1) + 1)*(grid_width*(TILE_WIDTH_CHARS+1) + 2) \
            	   + card.y*(TILE_WIDTH_CHARS+1) + bias] = '_';
         // Value
         buffer[(card.x*(TILE_HEIGHT_CHARS+1) + TILE_HEIGHT_CHARS)*(grid_width*(TILE_WIDTH_CHARS+1) + 2) \
         		+ card.y*(TILE_WIDTH_CHARS+1) + TILE_WIDTH_CHARS] = (std::to_string(card.value % 10))[0];
         buffer[(card.x*(TILE_HEIGHT_CHARS+1) + TILE_HEIGHT_CHARS)*(grid_width*(TILE_WIDTH_CHARS+1) + 2) \
         		+ card.y*(TILE_WIDTH_CHARS+1) + TILE_WIDTH_CHARS-1] = (std::to_string(card.value / 10))[0];
         // Advantage points (if there is any)
         if(card.advantage > 0)
         {
            buffer[(card.x*(TILE_HEIGHT_CHARS+1) + TILE_HEIGHT_CHARS)*(grid_width*(TILE_WIDTH_CHARS+1) + 2) \
                   + card.y*(TILE_WIDTH_CHARS+1) + 2] = (std::to_string(card.advantage % 10))[0];
            buffer[(card.x*(TILE_HEIGHT_CHARS+1) + TILE_HEIGHT_CHARS)*(grid_width*(TILE_WIDTH_CHARS+1) + 2) \
            	   + card.y*(TILE_WIDTH_CHARS+1) + 1] = (std::to_string(card.advantage / 10))[0];
         }
         // Overwhelmed indicator
         if(card.is_overwhelmed)
            buffer[(card.x*(TILE_HEIGHT_CHARS+1) + 2)*(grid_width*(TILE_WIDTH_CHARS+1) + 2) \
            	   + card.y*(TILE_WIDTH_CHARS+1) + TILE_WIDTH_CHARS] = '!';
         // Ability exhaustion
         if(!card.can_move)
            if(!card.can_attack)
               buffer[(card.x*(TILE_HEIGHT_CHARS+1) + 2)*(grid_width*(TILE_WIDTH_CHARS+1) + 2) \
               		  + card.y*(TILE_WIDTH_CHARS+1) + 1] = 'X';
            else
               buffer[(card.x*(TILE_HEIGHT_CHARS+1) + 2)*(grid_width*(TILE_WIDTH_CHARS+1) + 2) \
               		  + card.y*(TILE_WIDTH_CHARS+1) + 1] = '~';
      }
   }
   std::cout << buffer << std::endl;

   std::cout << "Turn " << turn_absolute + 1 << "\n";
   std::cout << "P1: $" << players[0].funds << "       " << players[0].points << " DP       " << players[0].hand_size << " in hand\n";
   std::cout << "P2: $" << players[1].funds << "       " << players[1].points << " DP       " << players[1].hand_size << " in hand\n";

   // HAND __________________________________________
   
   std::cout << "HAND: " << hand.size() \
               << "/??    DECK: " << players[id].deck_size \
               << "    DISCARD: " << players[id].discard_size << "\n";
   std::cout << "HAND:____________________________\n";
   std::cout << "YOU HAVE $" << players[id].funds << ":\n\n";

   for(int i=0; i<hand.size(); i++)
   {
      std::cout << hand[i].global_id << std::endl;
      Description_generator::Card_descr original = desc_gen.get_card_instance(hand[i].global_id);
      std::cout  << "[" << i << "] $" << hand[i].cost << " " << original.name << " (";
      			 (hand[i].value >= 0 ? std::cout << hand[i].value : std::cout << "T") \
      			 << ")" << "\n|   " << original.ability_text << "\n\n";
   }
   
   std::cout << "CONTRACTS:_______________________\n";
   
   // CONTRACTS _____________________________________
   for(Card_info card : active_cards)
   {
      if(card.type == CTYPE_CONTRACT && card.owner_id == id)
      {
         Description_generator::Card_descr origin = desc_gen.get_card_instance(card.global_id);
         std::cout << origin.name << " (" << card.value << ")\n";
      }
   }
   std::cout << std::endl;

};

Commander::Order CLI_commander::get_action()
{
   Order action = {};
   std::string buffer;

while(true)
   {
      std::cout << "PLAYER " << id << " GOES \n"; 
      std::cout << "PENDING ACTION: (P)ASS - (M)OVE - (A)TTACK - (D)EPLOY" << std::endl;
      std::cin >> buffer;
      
      if(buffer.size() != 1)
      {
         std::cout << "Invalid input;" << std::endl;
         continue;
      }

      switch (buffer[0])
      {
      case 'p': case 'P': //Pass
         action.type = Order::ORD_PASS;
         return action;
         break;

      case 'm': case 'M': //Move
         action.args.resize(4);
         action.type = Order::ORD_MOVE;
         std::cout << "SPECIFY RECEIVER COORDINATES:" << std::endl;
         std::cin >> action.args[0] >> action.args[1];
         std::cout << "SPECIFY DESTINATION COORDINATES:" << std::endl;
         std::cin >> action.args[2] >> action.args[3];
         return action;
         break;

      case 'a': case 'A': //Attack
         action.args.resize(4);
         action.type = Order::ORD_ATTACK;
         std::cout << "SPECIFY RECEIVER COORDINATES:" << std::endl;
         std::cin >> action.args[0] >> action.args[1];
         std::cout << "SPECIFY TARGET COORDINATES:" << std::endl;
         std::cin >> action.args[2] >> action.args[3];
         return action;
         break;

      case 'd': case 'D': //Deploy
         action.args.resize(1);
         action.type = Order::ORD_PLAY;
         std::cout << "SPECIFY CARD NO.:" << std::endl;
         std::cin >> action.args[0];
         if(action.args[0] >= 0 && action.args[0] < hand.size() && hand[action.args[0]].type == CTYPE_UNIT)
         {
            action.args.resize(3);
            std::cout << "SPECIFY DEPLOYMENT COORDINATES:" << std::endl;
            std::cin >> action.args[1] >> action.args[2];
         }
         return action;
         break;
      
      
      default:
         std::cout << "Invalid input;" << std::endl;
         continue;
      }
   }
}

void CLI_commander::handle_controller_event(const Event& event)
{
   //enum invalidAction {INVORD_NONE, INVTYPE, NOARGS, INVARGS, PERMISSION, NOSELECT, NOTARGET, EXHAUSTED, NOFUNDS};
   switch(event.type)
   {
      case Event::EV_ORDER_INVALID:
      switch (event.data[0])
      {
         case Order::INVORD_NONE:
            std::cout << "\nFalse Alarm...\n" << std::endl;
            break;

         case Order::INVORD_INVTYPE:
            std::cout << "\nInvalid command type...\n" << std::endl;
            break;

         case Order::INVORD_INVARGS:
            std::cout << "\nInvalid argument(s)...\n" << std::endl;
            break;

         case Order::INVORD_PERMISSION:
            std::cout << "\nYou don't have permission...\n" << std::endl;
            break;

         case Order::INVORD_NOSELECT:
            std::cout << "\nNo unit has been selected...\n" << std::endl;
            break;

         case Order::INVORD_NOTARGET:
            std::cout << "\nNo target has been specified...\n" << std::endl;
            break;

         case Order::INVORD_EXHAUSTED:
            std::cout << "\nOption exhausted...\n" << std::endl;
            break;
         
         case Order::INVORD_NOFUNDS:
            std::cout << "\nInsufficient funds...\n" << std::endl;
            break;

         case Order::INVORD_UNKNOWN:
            std::cout << "\nAn unknown error occurred...\n" << std::endl;
            break;

         default:
            std::cout << "\nAn even less known error occurred...\n" << std::endl;
            break;
         }
         break;
      case Event::EV_GAME_WON_BY:
         if(event.data[0] == id)
         {
            std::cout << "You win!" << std::endl;
         }
         else
         {
            std::cout << "You lose!" << std::endl;
         }
         break;
   }

}

void CLI_commander::apply_updates()
{
   if(turn == id)
      render_UI();
}

void CLI_commander::highlight_tile_bold(std::string &buffer, int g_width, int x, int y)
{
    // corners
    buffer[x*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1)] = '+';
    buffer[(x+1)*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1)] = '+';
    buffer[x*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + (y+1)*(TILE_WIDTH_CHARS+1)] = '+';
    buffer[(x+1)*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + (y+1)*(TILE_WIDTH_CHARS+1)] = '+';

    // vertical border
    for(int bias = 1; bias < (TILE_HEIGHT_CHARS+1); bias++)
    {
       buffer[(x*(TILE_HEIGHT_CHARS+1) + bias)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1)] = '|';
       buffer[(x*(TILE_HEIGHT_CHARS+1) + bias)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + (y+1)*(TILE_WIDTH_CHARS+1)] = '|'; 
    }

    // horizontal border
    for(int bias = 1; bias < (TILE_WIDTH_CHARS+1); bias++)
    {
       buffer[x*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1) + bias] = '-';
       buffer[(x+1)*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1) + bias] = '-'; 
    }
}

void CLI_commander::highlight_tile_subtle(std::string &buffer, int g_width, int x, int y)
{
	// corners
    buffer[x*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1)] = '+';
    buffer[(x+1)*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1)] = '+';
    buffer[x*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + (y+1)*(TILE_WIDTH_CHARS+1)] = '+';
    buffer[(x+1)*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + (y+1)*(TILE_WIDTH_CHARS+1)] = '+';

    // vertical border
    for(int bias = 1; bias < (TILE_HEIGHT_CHARS+1); bias++)
    {
       buffer[(x*(TILE_HEIGHT_CHARS+1) + bias)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1)] = '.';
       buffer[(x*(TILE_HEIGHT_CHARS+1) + bias)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + (y+1)*(TILE_WIDTH_CHARS+1)] = '.'; 
    }

    // horizontal border
    // for(int bias = 1; bias < (TILE_WIDTH+1); bias++)
    // {
    //    buffer[x*(TILE_HEIGHT+1)*(width*(TILE_WIDTH+1) + 2) + y*(TILE_WIDTH+1) + bias] = '.';
    //    buffer[(x+1)*(TILE_HEIGHT+1)*(width*(TILE_WIDTH+1) + 2) + y*(TILE_WIDTH+1) + bias] = '.'; 
    // }
}

void CLI_commander::highlight_tile_funky(std::string &buffer, int g_width, int x, int y)
{
	// corners
    buffer[x*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1)] = '%';
    buffer[(x+1)*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1)] = '%';
    buffer[x*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + (y+1)*(TILE_WIDTH_CHARS+1)] = '%';
    buffer[(x+1)*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + (y+1)*(TILE_WIDTH_CHARS+1)] = '%';

    // vertical border
    for(int bias = 1; bias < (TILE_HEIGHT_CHARS+1); bias++)
    {
       buffer[(x*(TILE_HEIGHT_CHARS+1) + bias)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1)] = '%';
       buffer[(x*(TILE_HEIGHT_CHARS+1) + bias)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + (y+1)*(TILE_WIDTH_CHARS+1)] = '%'; 
    }

    // horizontal border
    for(int bias = 1; bias < (TILE_WIDTH_CHARS+1); bias++)
    {
       buffer[x*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1) + bias] = '%';
       buffer[(x+1)*(TILE_HEIGHT_CHARS+1)*(g_width*(TILE_WIDTH_CHARS+1) + 2) + y*(TILE_WIDTH_CHARS+1) + bias] = '%'; 
    }
}

/*

CONTROLS TODO
tab - toggle hand
shift - see additional game data
wasd - move cursor 

rclick deployed cards - see descriptions

scroll card descriptions

*/