#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// This program was written by Aaron Lin on 21/03/19
// Note: No for loops were allowed
/*  Rules:
    -   4 players
    -   Each player starts with 10 cards
    -   40 cards total
    -   Largest is 49, smallest is 10
    -   In this case it is only playing by yourself
    -   Each round:
            o   Each player plays a card
            o   Starts from player in position 0 and goes clockwise
            o   If first player plays a composite:
                    ->  Rest of the players need to play a cocomposite - card with common factor with first card
                    ->  No cocomposite - Can play any card
                    ->  Largest cocomposite loses and receives a point
                    ->  If loser receives the Douglas (42) - receives another point
            o   If first player plays a prime:
                    ->  Rest of the players need to play a prime
                    ->  No primes - Can play any card
                    ->  Largest prime loses and receives a point
            o   Player who loses the round starts the next round
    -   Loser is the player with most points

    Strategy:
    -   Play the highest prime/cocomposite that is small enough to avoid losing
    -   Play the smallest prime/cocomposite when above is not possible
    -   Play the highest composite when there is no cocomposite of first card in your hand

    Aim:
    -   Lose the least rounds
*/

#define N_CARDS              40
#define N_CARDS_INITIAL_HAND 10
#define N_PLAYERS             4
#define N_CARDS_DISCARDED     3

#define CARD_MIN             10
#define CARD_MAX             49

#define N_CARDS_ROUND         4
#define DOUGLAS              42


void print_player_name(void);
void add_to_hand(int hand[]);
void choose_discards(int hand[]);
void choose_card_to_play(int hand[], int num_cards_hand);
int is_prime(int num);
int is_cocomp(int num_played, int hand[], int num_cocomps);
int smallest_cocomposite(int largest_played, int first_num_played, int hand[], int num_cocomps);
int largest_cocomposite(int largest_played, int first_num_played, int hand[], int num_cocomps);
void order_list(int list[], int seed, int size);
int player_lost (int played_list[], int size);
void sort_ascending (int hand[], int size);

// Global Variables
int pos;
int history[N_CARDS];
int douglas_played = 0;
char *player_name = "FishyFlop";
int order[N_PLAYERS];
int points[N_PLAYERS] = {0};

int main(void) {

    print_player_name();
    // Initialises an empty hand of size 10
    int hand[N_CARDS_INITIAL_HAND];
    int rounds = 0;
    printf("Which position do you want to sit in (0-3): ");
    scanf("%d", &pos);
    
    int i = 0;
    while (i < N_PLAYERS) {
        if (i == pos) {
            // P == Player
            order[i] = 'P';
        } else {
            order[i] = i;
        }
        i++;
    }
    add_to_hand(hand);
    choose_discards(hand);

    while (rounds < 10) {
        printf("\n");

        // Initialises: No. cards in hand // No. of cards played // Position on table (determines order)
        int num_cards_hand = N_CARDS_INITIAL_HAND - rounds;
        int num_cards_played = 4 * rounds;
        order_list(order, pos, N_PLAYERS);
        
        // Preliminary information
        printf("# Cards in hand: %d\n", num_cards_hand);
        printf("# Cards played: %d\n", num_cards_played);
        printf("Position in table: %d\n", pos);

        // Printing cards in hand
        printf("\nHAND:\n");
        printf("   ");
        int card_num = 0;
        while (card_num < N_CARDS_INITIAL_HAND - rounds) {
            printf(" %d", hand[card_num]);
            card_num++;
        }
        printf("\n\n");

        choose_card_to_play(hand, num_cards_hand);
        // Player who lost the last game gains a point
        points[pos]++;
        
        rounds++;
    }
    printf("\nPoints:\n");
    int p = 0;
    int loser = -1;
    int most_points = 0;
    while (p < N_PLAYERS) {
        if (order[p] == 'P') {
            printf("    %s - %d points\n", player_name, points[p]);
        } else {
            printf("    Player %d - %d points\n", order[p], points[p]);
        }

        if (points[p] > most_points) {
            most_points = points[p];
            loser = p;
        }
        p++;
    }
    printf("\n");

    if (loser == 'P') {
        printf("%s has lost the game...", player_name);
    } else {
        printf("Player %d has lost the game...", loser);
    }

    return 0;
}

// Function that prints the player name
void print_player_name(void) {
    printf("Current Player: %s\n", player_name);
}

int random_num_not_in_hand(int hand[], int size) {
    int found = 0;

    while (!found) {
        int random = (rand()%(CARD_MAX-CARD_MIN)) + CARD_MIN;
        int i = 0;
        while (i < size) {
            // Check if there is a match
            if (random == hand[i]) {
                break;
            }
            i++;
        }
        // if i == size then traversed whole array without a match
        if (i == size) {
            found = 1;
            return random;
        }
    }
}

void add_to_hand(int hand[]) {
    int i = 0;
    
    // Adds 10 cards to your hand
    printf("Add 10 cards to your hand: ");
    while (i < N_CARDS_INITIAL_HAND) {
        
        // Adds card into array (your 'hand')
        scanf("%d", &hand[i]);
        int n = 0;
        int card_in_hand = 0;
        while (n < i) {
            if (hand[i] == hand[n]) {
                printf("%d is in hand... Please enter another number: ", hand[i]);
                card_in_hand = 1;
                break;
            }
            n++;
        }
        if (card_in_hand) {
            continue;
        }

        i++;
    }

    sort_ascending(hand, N_CARDS_INITIAL_HAND);
}

// At the start of the game you will choose to discard 3 cards that will be passed onto the player to the left
void choose_discards(int hand[]) {
    
    // Set i as the end of the list
    int i = N_CARDS_INITIAL_HAND - 1;
    int num_discarded = 0;
    
    // Discards 3 largest numbers in hand except for DOUGLAS
    while (num_discarded < 3) {
        printf("Discarding... %d\n", hand[i - num_discarded]); 
        num_discarded += 1;
    }

    // Assume person to the right discards three largest cards not in your hand
    int n = CARD_MAX;
    int cards_replaced = 0;
    int m = N_CARDS_INITIAL_HAND - 1;
    while (cards_replaced < 3) {
        // If the number is in your hand
        if (n == hand[m]) {
            n--;
        } else {
            hand[m] = n;
            printf("Adding... %d\n", hand[m]);
            m--;
            n--;
            cards_replaced++;
        }
    }
}


void choose_card_to_play(int hand[], int num_cards_hand) {

    // # Primes played
    int prime_played = 0;

    // # Primes and Cocomposites in hand
    int num_prime = 0;
    int num_cocomp = 0;
    
    // Splitting up primes and cocomposites in the hand
    int cocomp_hand[num_cards_hand];
    int prime_hand[num_cards_hand];

    // Do I have the douglas
    int have_douglas = 0;
    
    int i = 0;
    while (i < num_cards_hand) {
        int card = hand[i];
        if (is_prime(card) == 1) {
            // Put card in prime_hand
            prime_hand[num_prime] = card;
            num_prime += 1;
        } else {
            // Put card in cocomposite hand
            cocomp_hand[num_cocomp] = card;
            // Checks for the DOUGLAS card (42) which is a cocomposite of most other cards
            if (card == DOUGLAS) {
                have_douglas = 1;
            }
            num_cocomp += 1;
        }
        i += 1;
    }
    
    // Will overcount by 1 at the end of each so this will ensure it is accurate
    num_prime -= 1;
    num_cocomp -= 1;

    // Reading cards previously played
    int played[N_CARDS_ROUND];
    int larger_than_douglas = 0;
    int largest_prime = 0;
    int largest_composite = 0;
    int douglas_in_round = 0;
    
    int order_nth = 0;
    int num_cards_played = 0;
    while (num_cards_played < pos && order[order_nth] != 'P') {
        played[num_cards_played] = random_num_not_in_hand(hand, num_cards_hand);
        printf("Player %d: Playing... %d\n", order[order_nth], played[num_cards_played]);
        order_nth++;
        
        if (is_prime(played[num_cards_played]) == 1) {
            prime_played = 1;
            if (played[num_cards_played] > largest_prime) {
                largest_prime = played[num_cards_played]; //largest prime played this round
            }
        } else {
            if (played[num_cards_played] > DOUGLAS) {
                larger_than_douglas = 1;
            } else if (played[num_cards_played] == DOUGLAS) {
                douglas_in_round = 1;
                douglas_played = 1;
            }
            if (played[num_cards_played] > largest_composite) {
                largest_composite = played[num_cards_played]; //largest composite played this round
            }
        }

        num_cards_played++;
    }

    // Reading cards played into history
    
    // Number of cards played = 4 x number of rounds played
    int num_card_history = N_CARDS_ROUND * (N_CARDS_INITIAL_HAND - num_cards_hand);
    i = 0;
    while (i < num_cards_played) {
        history[num_card_history - 1] = played[i];
        num_card_history++;
        i++;
    }


    // Choose the card to play
    // Note: The round follows the first card played
    //       If first card is prime/cocomposite, other players must play the same
    //       If you do not have cocomposite of first card or no primes when first card was a prime
    //       can play any card in hand - opportunity to get rid of big cards
    // Winning condition: Player with the largest cocomposite or prime (whichever first person plays) gains a point
    //                    Player with most points at end of the game loses 
    //                    Player who loses the round with a douglas gains 2 points

    // If you are starting
    if (num_cards_played == 0) {
        
        // If you still have composites - play a cocomposite
        if (num_cocomp >= 0) { 
            int played_a_card = 0;
            i = 0;
            while (i < num_cocomp) {
                // If the largest cocomposite is smaller than the douglas, play it
                if (cocomp_hand[num_cocomp - i] < DOUGLAS) {
                    printf("%s: Playing... %d\n", player_name, cocomp_hand[num_cocomp - i]);
                    played[num_cards_played] = cocomp_hand[num_cocomp - i];
                    num_cocomp -= 1;
                    // Breaks the loop
                    i = num_cocomp;
                    played_a_card = 1;
                }
                i += 1;
            }
            // Haven't played a card yet largest cocomposite > douglas
            if (played_a_card == 0) {
                // Plays smallest composite
                printf("%s: Playing... %d\n", player_name, cocomp_hand[0]);
                played[num_cards_played] = cocomp_hand[0];
                num_cocomp -= 1;
            }
        // No more cocomposites
        } else {
            printf("%s: Playing... %d\n", player_name, prime_hand[0]); // Prints smallest prime
            played[num_cards_played] = prime_hand[0];
            num_prime -= 1;
        }
    // You are NOT starting
    } else {
        // This is the value of first card played
        int first_card = played[0];
        // Is the first card a prime?
        int first_card_prime = is_prime(first_card);
        // First card is a prime and you still have primes to play
        if (first_card_prime == 1 && num_prime >= 0) {
            //play the largest prime < largest prime played by others this round
            i = 0;
            int played_a_card = 0;
            while (i < num_prime) {
                // Plays biggest prime < the largest prime played
                if (prime_hand[num_prime - i] < largest_prime) {
                    printf("%s: Playing... %d\n", player_name, prime_hand[num_prime - i]);
                    played[num_cards_played] = prime_hand[num_prime - i];
                    //breaks the loop
                    i = num_prime;
                    num_prime -= 1;
                    played_a_card = 1;
                }
                i += 1;
            }
            // If you still have not played a card - play smallest prime
            if (played_a_card == 0) {
                printf("%s: Playing... %d\n", player_name, prime_hand[0]);
                played[num_cards_played] = prime_hand[0];
                num_prime -= 1;
            }   
        // First card is a composite
        } else {
            //Finds whether there are cocomposites of the first card in your hand (i.e. has common factors)
            int is_cocom = is_cocomp(first_card, cocomp_hand, num_cocomp+1);
            //There is a cocomposite in your hand
            if (is_cocom == 1) {
                int smallest_cocom = smallest_cocomposite(largest_composite, first_card, cocomp_hand, num_cocomp+1);
                int largest_cocom =  largest_cocomposite(largest_composite, first_card, cocomp_hand, num_cocomp+1);
                // If you have the douglas and there is card larger than it - safe to play douglas
                if (have_douglas == 1 && larger_than_douglas == 1) {
                    //Prints DOUGLAS
                    printf("%s: Playing... %d\n", player_name, DOUGLAS);
                    played[num_cards_played] = DOUGLAS;
                    num_cocomp -= 1;
                } else if (douglas_in_round == 0 && pos == 3) {//DOUGLAS not played this round and you're playing last
                    printf("%s: Playing... %d\n", player_name, largest_cocom); //play largest cocomposite
                    played[num_cards_played] = largest_cocom;
                    num_cocomp -= 1;
                } else {
                    printf("%s: Playing... %d\n", player_name, smallest_cocom); //Plays smallest cocomposite less than first card
                    played[num_cards_played] = smallest_cocom;
                    num_cocomp -= 1;
                }
               
            } else { // There isnt a cocomposite in your hand
                if ((prime_played == 1 && num_prime >= 0) || num_cocomp < 0) { //If prime has been played, print biggest prime
                    printf("%s: Playing... %d\n", player_name, prime_hand[num_prime]); //largest prime
                    played[num_cards_played] = prime_hand[num_prime];
                    num_prime -= 1;
                } else {
                    printf("%s: Playing... %d\n", player_name, cocomp_hand[num_cocomp]); // Prints largest cocomposite
                    played[num_cards_played] = cocomp_hand[num_cocomp];
                    num_cocomp -= 1;
                }
            }
        }
    }
    order_nth++;
    num_cards_played++;

    // Remaining players take their turns
    while (num_cards_played < N_CARDS_ROUND) {
        int random = random_num_not_in_hand(hand, num_cards_hand);
        int n = 0;
        while (n < num_card_history) {
            if (random == history[n]) {
                break;
            }
            n++;
        }
        if (n == num_card_history) {
             printf("Player %d: Playing... %d\n", order[order_nth], random);
             played[num_cards_played] = random;
             num_cards_played++;
             order_nth++;
        }
    }

    pos = player_lost(played, N_CARDS_ROUND);
    // Add an extra point to loser if douglas played
    if (have_douglas || douglas_in_round) points[pos]++;
}

// A function to check if a number is a prime
int is_prime(int num) {

    int count = 2;
    int is_prime = 1;
    
    while (count <= num/2) {
        if (num % count == 0) {
            is_prime = 0;
        }
        count += 1;
    }
    return is_prime;
    
}

// A function to check if two numbers are cocomposites
int is_cocomp(int num_played, int hand[], int num_cocomps) {
    
    int count = 2;
    int i = 0;
    int is_cocomp = 0;

   // int factors[10]; // Max 3 factors
   
    while (count < num_played) {
        i = 0;
        while (i < num_cocomps) {
            if (num_played % count == 0 && hand[i] % count == 0) { // check if it is a factor of both the first card and the number in your hand
                is_cocomp = 1;
            }
            i += 1;
        }
        count += 1;
    }
    
    return is_cocomp;
}

// A function to find the smallest cocomposite in your hand
int smallest_cocomposite(int largest_played, int first_num_played, int hand[], int num_cocomps) {
    // returns smallest cocomposite < first card (num_played)
    int count = 2;
    int i = 0;
    int factor = 1;
    int smallest_cocomposite = 0;
    int cocomp_set = 0;
    while (count < largest_played) {
        i = 0;
        while (i < num_cocomps) {
            if (first_num_played % count == 0 && hand[i] % count == 0) {
                if (hand[i] < largest_played && hand[i] > smallest_cocomposite && hand[i] != DOUGLAS) {// Makes sure the cocomposite in hand is smaller than the largest number played but also not the Douglas
                    smallest_cocomposite = hand[i];
                    cocomp_set = 1;
                }
            }
            i += 1;
        }
        count += 1;
    }
    count = 2;
    if (cocomp_set == 0) {
        while (count < largest_played) {
            i = 0;
            while (i < num_cocomps) {
                if (first_num_played % count == 0 && hand[i] % count == 0) {
                    smallest_cocomposite = hand[i];
                }
                i += 1;
            }
            count += 1;
        }
    }
    
    return smallest_cocomposite;

}

// A function to find the largest cocomposite in your hand
int largest_cocomposite(int largest_played, int first_num_played, int hand[], int num_cocomps) {
    // returns largest cocomposite < first card (num_played)
    int count = 2;
    int i = 0;
    int factor = 1;
    int largest_cocomposite = 0;
    int cocomp_set = 0;
    while (count < largest_played) {
        i = 0;
        while (i < num_cocomps) {
            if (first_num_played % count == 0 && hand[i] % count == 0) {
                if (hand[i] > largest_cocomposite) {
                    largest_cocomposite = hand[i];
                    cocomp_set = 1;
                }
            }
            i += 1;
        }
        count += 1;
    }
   
    return largest_cocomposite;

}

void order_list(int list[], int seed, int size) {
    int i = 0;
    int new_list[size];
    while (i < size) {
        if (i-seed < 0) {
            new_list[size + (i - seed)] = list[i];
        } else {
            new_list[i-seed] = list[i];
        }
        i++;
    }

    list = new_list;
}

int player_lost (int played_list[], int size) {
    int p = 0;
    int largest = 0;
    int loser = -1;
    // Determining the largest num played and who played it - i.e. who lost
    while (p < size) {
        if (played_list[p] > largest) {
            largest = played_list[p];
            loser = p;
        }
        p++;
    }

    return loser;
}

//  Insertion sort on list:
//      -   Sorted array (left side) | Unsorted array (right side)
//      -   Everything < i is sorted - compare i to everything before to check if ordered
void sort_ascending (int hand[], int size) {
    // Checks if each one is in the right order via insertion sort
    int i = 1;
    while (i < N_CARDS_INITIAL_HAND) {
        int num = hand[i];
        int n = i - 1;
        int order_changed = 0;

        while (n >= 0) {
            // if hand[i] < hand[i-1] - where hand[0] to hand[i-1] is already sorted
            if (hand[n] > num) {
                hand[n+1] = hand[n];
                order_changed = 1;
            } else break;
            n--;
        }

        if (order_changed) {
            hand[n+1] = num;
        }

        i++;
    }
    
}