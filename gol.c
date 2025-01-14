
/*
 * Swarthmore College, CS 31
 * Copyright (c) 2023 Swarthmore College Computer Science Department,
 * Swarthmore PA
 */

/*
 * Date: 12/11/24
 * Nick Matese
 * This program implements Conway's Game of Life. 
 */
/*
 * To run:
 * ./gol file1.txt  0  # run with config file file1.txt, do not print board
 * ./gol file1.txt  1  # run with config file file1.txt, ascii animation
 * ./gol file1.txt  2  # run with config file file1.txt, ParaVis animation
 *
 */
#include <pthreadGridVisi.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "colors.h"

/****************** Definitions **********************/
/* Three possible modes in which the GOL simulation can run */
#define OUTPUT_NONE   (0)   // with no animation
#define OUTPUT_ASCII  (1)   // with ascii animation
#define OUTPUT_VISI   (2)   // with ParaVis animation

/* Used to slow down animation run modes: usleep(SLEEP_USECS);
 * Change this value to make the animation run faster or slower
 */
//#define SLEEP_USECS  (1000000)
#define SLEEP_USECS    (100000)

/* A global variable to keep track of the number of live cells in the
 * world (this is the ONLY global variable you may use in your program)
 */
static int total_live = 0;
/* declare a mutex: initialize in main */
static pthread_mutex_t my_mutex;

/* declare a barrier: initialize in main */
static pthread_barrier_t my_barrier;

/* This struct represents all the data you need to keep track of your GOL
 * simulation.  Rather than passing individual arguments into each function,
 * we'll pass in everything in just one of these structs.
 * this is passed to play_gol, the main gol playing loop
 *
 * NOTE: You will need to use the provided fields here, but you'll also
 *       need to add additional fields. (note the nice field comments!)
 * NOTE: DO NOT CHANGE THE NAME OF THIS STRUCT!!!!
 */
struct gol_data {
    
    //Fields of our gol_data struct
    int rows;  // the row dimension
    int cols;  // the column dimension
    int iters; // number of iterations to run the gol simulation
    int output_mode; // set to:  OUTPUT_NONE, OUTPUT_ASCII, or OUTPUT_VISI
    int num_threads; //# of threads
    int row_or_col; //which allocation style
    int printinfo; // determine if allocation info is printed
    int data_per_thread; //how many rows or columns does each thread handle
    int extra_data; //number of rows or columns left over from the even partioning
    int thread_id; //which thread is this?
    int start_index; //each threads start
    int end_index; //each threads end

    //the base and next arrays of our board
    int *base_arr;
    int *next_arr;

    /* fields used by ParaVis library (when run in OUTPUT_VISI mode). */
    // NOTE: DO NOT CHANGE their definitions BUT USE these fields
    visi_handle handle;
    color3 *image_buff;
};


/****************** Function Prototypes **********************/

/* the main gol game playing loop (prototype must match this) */
void *play_gol(void *args);

/* init gol data from the input file and run mode cmdline args */
int init_game_data_from_args(struct gol_data *data, char **argv);

/* print board to the terminal (for OUTPUT_ASCII mode) */
void print_board(struct gol_data *data, int round);

/* check for function errors*/
void check_error(int ret);

/* do necessary animation step*/
void animation_action(struct gol_data *data, int output_mode, int round);

/* find number of neighbors*/
int get_neighbors(struct gol_data *data, int x, int y);

/* returns 1 if alive, 0 if dead based on num neighbors*/
int alive_or_dead(int cell_status, int num_neighbors);

/* use updated data to set colors for visualization */
void update_colors(struct gol_data *data);

/* partition threads */
void partition_threads(struct gol_data *data);




/**************************************************************/


/************ Definitions for using ParVisi library ***********/
/* initialization for the ParaVisi library (DO NOT MODIFY) */
int setup_animation(struct gol_data* data);
/* register animation with ParaVisi library (DO NOT MODIFY) */
// int connect_animation(void (*applfunc)(struct gol_data *data),
//         struct gol_data* data);
/* name for visi (you may change the string value if you'd like) */
static char visi_name[] = "GOL ʕ•́ᴥ•̀ʔっ♡";
/**************************************************************/


/************************ Main Function ***********************/
int main(int argc, char **argv) {

    int ret;
    struct gol_data data;
    double secs;
    struct timeval start_time, stop_time; 


  // Initialize the mutex
    if (pthread_mutex_init(&my_mutex, NULL)) { 
        printf("pthread_mutex_init error\n");
        exit(1);
    }
    

  

    /* check number of command line arguments */
    if (argc < 3) {
        printf("usage: %s <infile.txt> <output_mode>[0|1|2]\n", argv[0]);
        printf("(0: no visualization, 1: ASCII, 2: ParaVisi)\n");
        exit(1);
    }

    /* Initialize game state (all fields in data) from information
     * read from input file */
    
    ret = init_game_data_from_args(&data, argv);

    // Initialize the barrier with num threads that will be synchronized

    if (pthread_barrier_init(&my_barrier, NULL, data.num_threads)) {
        printf("pthread_barrier_init error\n");
        exit(1);
    }


    /* Invoke play_gol in different ways based on the run mode */ 
    if (data.output_mode == OUTPUT_NONE) {  // run with no animation
        ret = gettimeofday(&start_time, NULL);
        check_error(ret);
        //partition and create threads to run gol
        partition_threads(&data);
        //play_gol(&data);
        ret = gettimeofday(&stop_time, NULL);
        check_error(ret);
    }
    else if (data.output_mode == OUTPUT_ASCII) { // run with ascii animation
        ret = gettimeofday(&start_time, NULL);
        check_error(ret);
        //partition and create threads to run gol
        partition_threads(&data);
        //play_gol(&data);
        ret = gettimeofday(&stop_time, NULL);
        check_error(ret);

        // clear the previous print_board output from the terminal:
        // (NOTE: you can comment out this line while debugging)

        //if (system("clear")) { perror("clear"); exit(1); }

        // NOTE: DO NOT modify this call to print_board at the end
        //       (it's to help us with grading your output)
        print_board(&data, data.iters);
    }
    else if (data.output_mode == OUTPUT_VISI) {  
        // OUTPUT_VISI: run with ParaVisi animation
        // tell ParaVisi that it should run play_gol
        setup_animation(&data);
        
        //partition and create threads to run gol
        partition_threads(&data);

        //MOVED INTO PARTITION THREADS
        // start ParaVisi animation
        //run_animation(data.handle, data.iters);
    } else {
        printf("Invalid output mode: %d\n", data.output_mode);
        printf("Check your game data initialization\n");
        exit(1);
    }

    //Timing
    if (data.output_mode != OUTPUT_VISI) {
        double ms, s; 
        secs = 0.0;
        // Compute the total runtime in seconds
        ms = (stop_time.tv_usec/1000000.0)-(start_time.tv_usec/1000000.0);
        s = (stop_time.tv_sec)-(start_time.tv_sec);
        secs = ms + s;
        /* Print the total runtime, in seconds. */
        // NOTE: do not modify these calls to fprintf
        fprintf(stdout, "Total time: %0.3f seconds\n", secs);
        fprintf(stdout, "Number of live cells after %d rounds: %d\n\n",
                data.iters, total_live);
    }

    // clean-up before exit
    free(data.base_arr);
    free(data.next_arr);

    return 0;
}
/**************************************************************/

/******************** Initialize Game Data  **************
 * argv: command line args
 *       argv[1]: name of file to read game config state from
 *       argv[2]: run mode
 *       argv[3]: number of thread
 *       argv[4]: row vs column parallelism
 *       argv[5]: whether to print allocation info
 * returns: 0 on success, 1 on error
 */
int init_game_data_from_args(struct gol_data *data, char **argv) {

    int  howmany, ret;
    FILE *infile;
    //open file, get file ptr
    infile = fopen(argv[1], "r");
    if (infile == NULL) {
        printf("Error: failed to open file: %s\n", argv[1]);
        exit(1);
    }

    //copy the game mode, atoi converts char to int
    data->output_mode = atoi(argv[2]);

    //copy the flag for row or cols
    data->row_or_col = atoi(argv[4]);
    //error check row or col
    if ((data->row_or_col != 0) && (data->row_or_col != 1)) {
        printf("argv[4] error, enter 0 or 1.\n");
        exit(1);
    }

    //copy the flag for determining if to print info
    data->printinfo = atoi(argv[5]);





    ret = fscanf(infile, "%d\n %d\n %d\n %d", &data->rows, &data->cols, &data->iters, &howmany);
    //check for file error
    if(ret != 4){
        printf("Improper file format\n");
        exit(1);
    }

    //Threads basic init
    //copy the number of threads
    data->num_threads = atoi(argv[3]);
    if (data->num_threads < 1) {
        printf("Need at least one thread to run.\n");
        exit(1);
    }
    if ((data->num_threads > data->rows) || (data->num_threads > data->cols)) {
        printf("Too many threads, run with equal or less than row/col size.\n");
        exit(1);
    }

    //find out the number of rows or columns to each thread
    if (data->row_or_col == 0) {
            data->data_per_thread = data->rows / data->num_threads;
            data->extra_data = data->rows  % data->num_threads;
        }
    else {
            data->data_per_thread = data->cols / data->num_threads;
            data->extra_data = data->cols % data->num_threads;
        }


    total_live = howmany; //set initial # of alive cells to the total_live global variable

    int *base_arr;       // a dynamically allocated "2D" array using 1 malloc

    base_arr = malloc(sizeof(int)*data->rows*data->cols); //malloc memory
    if (!base_arr) { //make sure malloc was succesful
        printf("malloc failed, check file format\n");
        exit(1);
    }
    
    //calculate total length of array
    int length = data->rows * data->cols;

    //initialize the array to 0
    for (int i = 0; i<length;i++) {
       base_arr[i] = 0;
    }
    

    //populate initial board with initial alive cells
    int count = 0;
    int i = 0;
    int j = 0; 
    while (count < howmany) {
        //get i and j
        ret = fscanf(infile, "%d %d\n", &i, &j);
        //check for file error
        if(ret != 2){
            printf("Improper file format. \n");
            exit(1);
    }
        //set the cell of i and j to 1
        base_arr[(i)*data->cols+j] = 1;
        count++;
    }


    //copy arr to struct
    data->base_arr = base_arr;

    int *next_arr;       // a dynamically allocated "2D" array using 1 malloc

    next_arr = malloc(sizeof(int)*data->rows*data->cols); //malloc memory
    if (!base_arr) { //make sure malloc was succesful
        printf("malloc failed\n");
        exit(1);
    }

    //copy next arr to struct
    data->next_arr = next_arr;

    //close file
    ret = fclose(infile);
    check_error(ret);


    return 0;
}

/**************************************************************/
/******************** Play Game of Life **********************
 * play_gol: The main simulation loop for Conway's Game of Life.
 * args: A pointer to a gol_data structure containing grid and thread information.
 *       Synchronizes threads using barriers and manages cell counts with mutex locks.
 * returns: NULL (used as a pthread-compatible thread function).
 ***************************************************************/


void *play_gol(void *args) {
    struct gol_data *data;  
    
    data = (struct gol_data *)args;

    int num_threads = data->num_threads;
    int cols = data->cols;
    int rows = data->rows;
    int iters = data->iters;
    int output_mode = data->output_mode;
    int round = 0;
    int *temp;
    int local_live_count = 0;
    int thread_num = data->thread_id;
    int row_start,row_end,col_start,col_end;

    if (data->printinfo == 1) {
        if(thread_num<num_threads+1) {
            if (data->row_or_col == 0) {
                printf("tid %4d: rows:  %5d:%-4d (%d) cols:  %5d:%-4d (%d)\n", thread_num,
                data->start_index, data->end_index, (data->end_index-data->start_index+1) , 0,cols-1, cols );
            } else {
                printf("tid %4d: rows:  %5d:%-4d (%d) cols:  %5d:%-4d (%d)\n", thread_num,
                0,rows-1, rows, data->start_index, data->end_index, (data->end_index-data->start_index+1) );
            }
        }
    }
    
    if(data->row_or_col == 0){
        row_start = data->start_index;
        row_end = data->end_index;
        col_start = 0;
        col_end = cols-1;
    } else {
        col_start = data->start_index;
        col_end = data->end_index;
        row_start = 0;
        row_end = rows-1;
    }


    pthread_barrier_wait(&my_barrier);


    //print initial board
    animation_action(data, output_mode, round);
    
    //increment round from 0 to 1
    round++;

    while (round <= iters) {
    
        if(data->thread_id == 1){
            total_live=0;
        }


        pthread_barrier_wait(&my_barrier);


        local_live_count = 0;
        for (int j = col_start; j <= col_end;j++) {
            for (int i = row_start; i <= row_end; i++) {
                    
                    //update next board, using our functions. We call alive our dead, with  get_neighbors called inside
                data->next_arr[(i)*cols+(j)] = alive_or_dead(data->base_arr[(i)*cols+(j)], (get_neighbors(data,i,j)));
                    
                    //update alive count if cell is alive
                local_live_count+=data->next_arr[(i)*data->cols+(j)];
            }
        }

        //using mutex lock to lock one threads actions
        pthread_mutex_lock(&my_mutex);
        //incriment global variable total_live
        total_live += local_live_count;
        
        pthread_mutex_unlock(&my_mutex);  
        //copy next array to base array
        temp = data->base_arr;
        data->base_arr = data->next_arr;
        data->next_arr = temp;
        
        
        //printf("total live: %d\n", total_live);

        //unlock the threads actions
        
        pthread_barrier_wait(&my_barrier);






        //do correct animation step based on output mode at end of round
        if (round < iters) {
        animation_action(data, output_mode, round);
        }
        
        //increment round
        round++;



    } 
    
    return NULL;



}

/**************************************************************/
/******************** Print Board **********************
 * print_board: Prints the current Game of Life board in ASCII format.
 * data: Pointer to a gol_data structure containing grid and state information.
 * round: The current round number of the simulation.
 * returns: void.
 ***************************************************************/

void print_board(struct gol_data *data, int round) {

    int i, j;

    /* Print the round number. */
    fprintf(stderr, "Round: %d\n", round);

    for (i = 0; i < data->rows; ++i) {
        for (j = 0; j < data->cols; ++j) {
            if (data->base_arr[(i)*data->cols+j] == 1) {
                fprintf(stderr, " @");
            }
            else {
                fprintf(stderr, " .");
            }
        }
        fprintf(stderr, "\n");
    }

    /* Print the total number of live cells. */
    fprintf(stderr, "Live cells: %d\n\n", total_live);
}
/**************************************************************/
///////////////////   HELPER FUNCTIONS     /////////////////////
/**************************************************************/

/******************** Error Checker **********************
 * check_error: Checks for errors in function calls.
 * ret: The return value to check.
 *       If ret != 0, the function prints an error message and exits.
 * returns: void.
 ***************************************************************/

void check_error(int ret) {
    if (ret != 0) {
        printf("Function error\n");
        exit(1);
    }
}

/******************** Run Animation Steps **********************
 * animation_action: Executes the animation step based on output mode.
 * data: Pointer to a gol_data structure.
 * output_mode: The mode of animation (1: ASCII, 2: ParaVisi).
 * round: The current simulation round.
 * returns: void.
 ***************************************************************/

void animation_action(struct gol_data *data, int output_mode, int round) {
    if (output_mode == 1) {
        //system("clear");
        if (data->thread_id == 1) {
            print_board(data, round);
        }
        usleep(SLEEP_USECS);
    }
    else if (output_mode == 2) {
        update_colors(data);
        draw_ready(data->handle);
        usleep(SLEEP_USECS);
    }        
}
/******************** Find Number of Neighbors **********************
 * get_neighbors: Counts live neighbors for a cell in the grid.
 * data: Pointer to a gol_data structure containing grid information.
 * x, y: Coordinates of the cell.
 * returns: Number of live neighbors (0-8).
 ***************************************************************/

int get_neighbors(struct gol_data *data, int x, int y) {
    //readability for our neighbors
    int cols = data->cols;
    int rows = data->rows;
    int num_neighbors = 0;

    //iterate through surrounding neighbors
    for (int ny = -1; ny<2; ny++) {
        for (int nx = -1; nx<2; nx++) {
            //do not count origin as neighbor
            if (ny != 0 || nx != 0) {
                // using mod to wrap around grid, check neighbors
                num_neighbors += data->base_arr[((nx+x+cols)%cols)*data->cols+((y+ny+rows)%rows)];

            }

        }
    }

    return num_neighbors;
} 
/******************** Game Logic Determinator **********************
 * alive_or_dead: Determines the next state of a cell.
 * cell_status: Current status of the cell (1: alive, 0: dead).
 * num_neighbors: Number of live neighbors for the cell.
 * returns: 1 if the cell is alive in the next generation, 0 otherwise.
 ***************************************************************/

int alive_or_dead(int cell_status, int num_neighbors) {
    // checking the cell status based on gol rules
    if ((num_neighbors == 2 && cell_status == 1) || num_neighbors == 3) {
        return 1;
    }
    else {
        return 0;
    }
   

}

/******************** Update ParaVisi Colors **********************
 * update_colors: Updates the ParaVisi animation buffer based on grid state.
 * data: Pointer to a gol_data structure containing grid and thread information.
 *       Updates the buffer colors for live and dead cells.
 * returns: void.
 ***************************************************************/

void update_colors(struct gol_data *data) {

    int rows, cols, index, buff_i, row_start,row_end,col_start,col_end;
    color3 *buff;

    buff = data->image_buff;  // just for readability
    rows = data->rows;
    cols = data->cols;

    if(data->row_or_col == 0){
        row_start = data->start_index;
        row_end = data->end_index;
        col_start = 0;
        col_end = cols-1;
    } else {
        col_start = data->start_index;
        col_end = data->end_index;
        row_start = 0;
        row_end = rows-1;
    }

    for (int j = col_start; j <= col_end;j++) {
        for (int i = row_start; i <= row_end; i++) {
        index = i*cols + j;
        // translate row index to y-coordinate value because in
        // the image buffer, (r,c)=(0,0) is the _lower_ left but
        // in the grid, (r,c)=(0,0) is _upper_ left.
        buff_i = (rows - (i+1))*cols + j;

        // update animation buffer, set to black if alive, colored if dead
        if (data->base_arr[index] == 1) {
            buff[buff_i] = c3_black;
        } 
        else {
            buff[buff_i] = colors[((data->thread_id)-1)%8];
        }
        }
    }  
}

/******************** Threading Implementation **********************
 * partition_threads: Divides grid data among threads and creates threads.
 * data: Pointer to a gol_data structure with the grid and thread settings.
 *       Assigns rows/columns to threads for processing and starts simulation.
 * returns: void.
 ***************************************************************/

void partition_threads(struct gol_data *data) {
    int num_threads = data->num_threads;
    struct gol_data *targs;
    int ret;
    pthread_t *tid; //pointer to future thread array

    tid = malloc(num_threads * sizeof(pthread_t));
    if (!tid) { perror("malloc: pthread_t array"); exit(1); }

    targs = malloc(sizeof(struct gol_data) * num_threads);
    if (!targs) { perror("malloc: int array"); exit(1); }

    //assign partition info while we create threads
    for(int i = 0; i<num_threads; i++) {
        targs[i] = *data;
        targs[i].thread_id = i+1;

        if (i < targs[i].extra_data) {
            targs[i].data_per_thread++;
        }    
        if (i == 0) {
            targs[i].start_index = i*targs[i].data_per_thread;
            targs[i].end_index = targs[i].start_index + targs[i].data_per_thread - 1;
        }
        else {
            targs[i].start_index =  targs[i-1].end_index+1;
            targs[i].end_index = targs[i].start_index + targs[i].data_per_thread - 1;
        }


        ret = pthread_create(&tid[i], NULL, play_gol, &targs[i]);
        if (ret) { perror("Error: pthread_create"); exit(1); }
    }
    
    // start ParaVisi animation
    if (data->output_mode == 2) {
        run_animation(data->handle, data->iters);
    }
    //join threads
    for(int i = 0; i<num_threads;i++) {
        pthread_join(tid[i],0);
    }

    pthread_mutex_destroy(&my_mutex);
    pthread_barrier_destroy(&my_barrier);

    free(tid);
    free(targs);
    targs = NULL;
    tid = NULL;


}


    //   if ParaVis animation:
/**************************************************************/
/***** START: DO NOT MODIFY THIS CODE *****/
/* initialize ParaVisi animation */
int setup_animation(struct gol_data* data) {
    /* connect handle to the animation */
    int num_threads = data->num_threads;
    data->handle = init_pthread_animation(num_threads, data->rows,
            data->cols, visi_name);
    if (data->handle == NULL) {
        printf("ERROR init_pthread_animation\n");
        exit(1);
    }
    // get the animation buffer
    data->image_buff = get_animation_buffer(data->handle);
    if(data->image_buff == NULL) {
        printf("ERROR get_animation_buffer returned NULL\n");
        exit(1);
    }
    return 0;
}

/* sequential wrapper functions around ParaVis library functions */
void (*mainloop)(struct gol_data *data);

void* seq_do_something(void * args){
    mainloop((struct gol_data *)args);
    return 0;
}

// int connect_animation(void (*applfunc)(struct gol_data *data),
//         struct gol_data* data)
// {
//     pthread_t pid;

//     mainloop = applfunc;
//     if( pthread_create(&pid, NULL, seq_do_something, (void *)data) ) {
//         printf("pthread_created failed\n");
//         return 1;
//     }
//     return 0;
// }
/***** END: DO NOT MODIFY THIS CODE *****/
