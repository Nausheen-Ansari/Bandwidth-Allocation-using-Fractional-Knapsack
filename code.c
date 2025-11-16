/**
 * @file bandwidth_allocator.c
 * @brief Bandwidth Allocation (Fractional Knapsack)
 *
 * This program implements the Fractional Knapsack algorithm to solve a network
 * bandwidth allocation problem.
 *
 * Problem:
 * An ISP has a limited amount of total bandwidth to distribute among
 * various users/tasks. Each task has a specific 'demand' (bandwidth
 * requested) and a 'priority' (value).
 *
 * Goal:
 * Allocate the bandwidth to maximize the total 'priority' value. Since
 * bandwidth is divisible, we can give fractional amounts.
 *
 * This is a Fractional Knapsack problem where:
 * - Knapsack Capacity = Total Available Bandwidth
 * - Items = Users / Tasks
 * - Item Weight = Bandwidth Demand
 * - Item Value = Priority
 *
 * The greedy strategy is to calculate the "value per weight" (Priority/Demand)
 * for each task, sort the tasks by this ratio in descending order, and
 * allocate bandwidth to tasks in that order.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @struct Task
 * @brief Represents a single user or task requesting bandwidth.
 */
struct Task {
    char name[100];     // Name of the user or task
    double demand;      // How much bandwidth is requested (Weight)
    int priority;       // Priority level of the task (Value)
    double ratio;       // The calculated priority-per-demand (Value/Weight)
    double allocated;   // The final allocated bandwidth (Output)
};

/**
 * @brief Comparison function for qsort.
 *
 * Sorts tasks in DESCENDING order based on their priority-to-demand ratio.
 * This is the core of the greedy strategy.
 *
 * @param a Void pointer to the first Task.
 * @param b Void pointer to the second Task.
 * @return int -1 if b's ratio is less than a's, 1 if b's ratio is greater, 0 if equal.
 */
int compareTasks(const void *a, const void *b) {
    // Cast the void pointers back to Task pointers
    struct Task *taskA = (struct Task *)a;
    struct Task *taskB = (struct Task *)b;

    // We want to sort in descending order (highest ratio first)
    if (taskB->ratio < taskA->ratio) {
        return -1; // a comes before b
    } else if (taskB->ratio > taskA->ratio) {
        return 1; // b comes before a
    } else {
        return 0; // ratios are equal
    }
}

/**
 * @brief Main function to run the bandwidth allocator.
 */
int main() {
    int numTasks;
    double totalBandwidth;
    double initialBandwidth; // To store the original total for percentage calculation
    double totalPriorityValue = 0.0; // Total value of allocated bandwidth

    // --- 1. Get Initial Inputs ---

    printf("--- Bandwidth Allocation (Fractional Knapsack) ---\n");
    printf("Enter the Total Available Bandwidth (e.g., 1000 Mbps): ");
    scanf("%lf", &totalBandwidth);
    initialBandwidth = totalBandwidth;

    printf("Enter the number of competing users/tasks: ");
    scanf("%d", &numTasks);

    if (numTasks <= 0) {
        printf("No tasks to allocate. Exiting.\n");
        return 0;
    }

    // --- 2. Dynamically Allocate Memory for Tasks ---
    // Using malloc for robustness instead of a VLA (Variable Length Array)
    struct Task *tasks = malloc(numTasks * sizeof(struct Task));
    if (tasks == NULL) {
        printf("Error: Failed to allocate memory for tasks.\n");
        return 1; // Exit with an error code
    }

    // --- 3. Get Task Details from User ---
    printf("\n--- Enter Task Details ---\n");
    for (int i = 0; i < numTasks; i++) {
        printf("Task #%d:\n", i + 1);

        printf("  Name: ");
        // Clear input buffer before reading string
        // (to consume the newline from the previous scanf)
        while (getchar() != '\n');
        fgets(tasks[i].name, 100, stdin);
        tasks[i].name[strcspn(tasks[i].name, "\n")] = 0; // Remove trailing newline

        printf("  Demand (Bandwidth requested): ");
        scanf("%lf", &tasks[i].demand);

        printf("  Priority (e.g., 1-100): ");
        scanf("%d", &tasks[i].priority);

        // Calculate the priority-to-demand ratio
        if (tasks[i].demand > 0) {
            tasks[i].ratio = (double)tasks[i].priority / tasks[i].demand;
        } else {
            // Handle division by zero (e.g., a task with 0 demand but some priority)
            // Give it the highest possible ratio to prioritize it,
            // though it won't consume bandwidth.
            tasks[i].ratio = (double)tasks[i].priority > 0 ? 1e9 : 0;
        }

        // Initialize allocated bandwidth to zero
        tasks[i].allocated = 0.0;
    }

    // --- 4. Sort Tasks by Ratio (The Greedy Step) ---
    // qsort will rearrange the 'tasks' array in place
    qsort(tasks, numTasks, sizeof(struct Task), compareTasks);

    // --- 5. Allocate Bandwidth (Fractional Knapsack Logic) ---
    printf("\n--- Processing Allocation (Highest Priority/Demand first) ---\n");
    for (int i = 0; i < numTasks; i++) {
        if (totalBandwidth <= 0) {
            printf("No more bandwidth to allocate. Stopping.\n");
            break; // No more bandwidth left
        }

        printf("Considering Task '%s' (Ratio: %.2f). Remaining Bandwidth: %.2f\n",
               tasks[i].name, tasks[i].ratio, totalBandwidth);

        if (tasks[i].demand <= totalBandwidth) {
            // Case 1: Full demand can be met
            tasks[i].allocated = tasks[i].demand;
            totalBandwidth -= tasks[i].demand;
            totalPriorityValue += tasks[i].priority;
            printf("  -> Allocated full demand (%.2f)\n", tasks[i].allocated);
        } else {
            // Case 2: Only a fraction of demand can be met
            tasks[i].allocated = totalBandwidth;
            
            // Calculate the fractional value
            double fraction = tasks[i].allocated / tasks[i].demand;
            totalPriorityValue += fraction * tasks[i].priority;

            totalBandwidth = 0; // All bandwidth is used up
            printf("  -> Allocated remaining bandwidth (%.2f)\n", tasks[i].allocated);
        }
    }

    // --- 6. Print the Final Allocation Table ---
    printf("\n--- Final Bandwidth Allocation Table ---\n\n");
    printf("Total Bandwidth: %.2f | Bandwidth Used: %.2f | Total Priority Value: %.2f\n",
           initialBandwidth, initialBandwidth - totalBandwidth, totalPriorityValue);
    
    printf("------------------------------------------------------------------------------------------------\n");
    printf("| %-20s | %-10s | %-15s | %-15s | %-20s |\n",
           "Task Name", "Priority", "Demand", "Allocated", "Share of Total (%)");
    printf("------------------------------------------------------------------------------------------------\n");

    // We print in the order of allocation (highest ratio first)
    // You could re-sort by name here if needed, but this order is often more useful.
    for (int i = 0; i < numTasks; i++) {
        double percentageShare = 0.0;
        if (initialBandwidth > 0) {
            percentageShare = (tasks[i].allocated / initialBandwidth) * 100.0;
        }

        printf("| %-20s | %-10d | %-15.2f | %-15.2f | %-20.2f%% |\n",
               tasks[i].name,
               tasks[i].priority,
               tasks[i].demand,
               tasks[i].allocated,
               percentageShare);
    }
    printf("------------------------------------------------------------------------------------------------\n");

    // --- 7. Clean up ---
    free(tasks); // Free the dynamically allocated memory
    tasks = NULL;

    return 0; // Success
}
