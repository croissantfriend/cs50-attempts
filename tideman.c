#include <cs50.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);
bool cycle(int current_pair);
int next_loser(int current_loser);
bool has_zero_col(void);
void print_locked(void);


int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    // Iterate through candidates
    for (int i = 0; i < candidate_count; i++)
    {
        // Compare candidate name with name vote cast for
        if (strcmp(name, candidates[i]) == 0)
        {
            // If names match, increment votes and return true
            ranks[rank] = i;
            return true;
        }
    }
    // If name vote cast for matches no candidate, return false
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    // Iterate through candidates
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            // Increment rank
            preferences[ranks[i]][ranks[j]]++;
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    // Initialize counter variable
    int n = 0;
    // Create variable to store max # of pairs
    int max_pairs = candidate_count * (candidate_count - 1) / 2;
    // Iterate through candidates
    for (int j = 0; j < candidate_count; j++)
    {
        for (int k = j + 1; k < candidate_count; k++)
        {
            // Compare pair rankings and update vars accordingly
            if (preferences[j][k] > preferences[k][j])
            {
                pairs[n].winner = j;
                pairs[n].loser = k;
                n++;
                pair_count++;
            }
            else if (preferences[k][j] > preferences[j][k])
            {
                pairs[n].winner = k;
                pairs[n].loser = j;
                n++;
                pair_count++;
            }
            // Return once all pairs accounted for
            if (n == max_pairs)
            {
                return;
            }
        }
    }
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    // Initialize counter variable
    int n = 0;
    // Initialize array to store sorted pairs intermediately
    pair sorted[pair_count];
    // Loop through pairs
    while (n != pair_count)
    {
        // Initialize variable to store highest vote count
        int highest_pref = 0;
        // Iterate through pairs
        for (int i = 0; i < pair_count; i++)
        {
            // Count votes
            int pref_count = preferences[pairs[i].winner][pairs[i].loser];
            // If current vote count higher than stored highest, update highest
            if (pref_count > highest_pref)
            {
                highest_pref = pref_count;
            }
        }
        // Iterate through pairs again
        for (int i = 0; i < pair_count; i++)
        {
            // Find highest vote count
            int pref_count = preferences[pairs[i].winner][pairs[i].loser];
            if (pref_count == highest_pref)
            {
                // Update intermediate results array
                sorted[n].winner = pairs[i].winner;
                sorted[n].loser = pairs[i].loser;
                // Set pair in prefs array to 0 so they won't be re-sorted
                preferences[pairs[i].winner][pairs[i].loser] = 0;
                preferences[pairs[i].loser][pairs[i].winner] = 0;
                // Increment counter
                n++;
            }
        }
    }
    // Store intermediate array in pairs[] and return
    for (int i = 0; i < pair_count; i++)
    {
        pairs[i].winner = sorted[i].winner;
        pairs[i].loser = sorted[i].loser;
    }
    return;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    // Create boolean to store whether a locking operation was skipped
    bool skipped = false;
    // Iterate through pairs
    for (int i = 0; i < pair_count; i++)
    {
        // Check locking wouldn't create a cycle, or a previous lock op was skipped
        if (!cycle(i) || skipped)
        {
            // Update locked array
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
        else
        {
            // If locked not updated, update skipped boolean
            skipped = true;
        }
        // Check results
        print_locked();
    }
    return;
    // bool new_locked[candidate_count][candidate_count];
    // for (int i = 0; i < candidate_count; i++)
    // {
    //     for (int j = 0; j < candidate_count; j++)
    //     {
    //         new_locked[i][j] = false;
    //     }
    // }
    // for (int i = 0; i < pair_count; i++)
    // {
    //     new_locked[pairs[i].winner][pairs[i].loser] = true;
    //     if (has_zero_col(new_locked))
    //     {
    //         locked[pairs[i].winner][pairs[i].loser] = true;
    //     }
    //     print_locked();
    // }
    // return;
}

// // Check that forming the next connection wouldn't create a cycle
bool cycle(int current_pair)
{
    // 1. find current_loser we're trying to connect to
    // 2, see if that loser wins against some other next_loser
    // 3. if we find a next_loser that is == to current_loser, don't lock

    // Store the loser of current node in a variable
    int source_loser;
    // Loop through pairs to find specific pair whose loser we're trying to connect to
    for (int i = 0; i < pair_count; i++)
    {
        if (i == current_pair)
        {
            source_loser = pairs[i].loser;
        }
    }
    // Initialize counter variable
    int n = 0;
    // Initialize variable to store potential loser to loser of current node
    int next_connection = -1;
    // Create variable to store current loser, starting from source loser
    int current_loser = source_loser;
    // Loop through pairs
    while (n < pair_count)
    {
        // Use helper function to find loser to current loser
        next_connection = next_loser(current_loser);
        // Compare loser to current loser with source loser
        if (next_connection == source_loser)
        {
            // If matched, return that cycle was found
            return true;
        }
        // Update current_loser and increment counter
        current_loser = next_connection;
        n++;
    }
    // If no cycle detected, return false
    return false;
}

// See who current loser beats
int next_loser(int current_loser)
{
    // Loop through pairs
    for (int j = 0; j < pair_count; j++)
    {
        // If current pair's winner is current loser:
        if (pairs[j].winner == current_loser)
        {
            // Return loser of current loser
            return pairs[j].loser;
        }
    }
    // If current_loser beats none, return -1
    return -1;
}

// Find the index of all-zero column in locked, representing source of graph
int find_zero_col(void)
{
    // Iterate through columns
    for (int i = 0; i < candidate_count; i++)
    {
        // Create variable to store sum of bools in col
        int col_sum = 0;
        // Iterate through rows
        for (int j = 0; j < candidate_count; j++)
        {
            // Update col_sum with bool value
            col_sum += locked[j][i];
        }
        // If col_sum == 0, return column #
        if (!col_sum)
        {
            return i;
        }
    }
    // If no all-zero columns, return -1
    return -1;
}

// Print the winner of the election
void print_winner(void)
{
    printf("%s\n", candidates[find_zero_col()]);
    return;
}

// Test function to print out the locked array
// to check that the lock_pairs function works and get
// the current state of the locked array
void print_locked(void)
{
    printf("-------------------------\n");
    printf("LOCKED ARRAY\n");
    printf("-------------------------\n");

    // This is the max width of the candidate names to make
    // sure that the 'grid' lines up. Shorter names will be
    // padded and longer names will be truncated.
    int width = 5;
    printf("%-*s   ", width, "");

    // Print out all the candidates column headers
    for (int i = 0; i < candidate_count; i++)
    {
        printf("%-5.5s ", candidates[i]);
    }
    printf("\n");

    // Print each candidate and if they have locked in their
    // win against another candidate
    for (int i = 0; i < candidate_count; i++)
    {
        printf("%-*.*s: ", width, width, candidates[i]);
        for (int j = 0; j < candidate_count; j++)
        {
            if (i == j)
            {
                // This result is actually false because this
                // is the candidate compared to themselves
                // but to make it easier to read the array
                // added in the dashes. If you don't like the
                // dashes uncomment the next line and comment
                // the final printf.
                // printf(" False");
                printf(" ---- ");
            }
            else
            {
                printf("%s", locked[i][j] ? "\033[1;32m True \033[0m" : " False");
            }
        }
        printf("\n");
    }
}
