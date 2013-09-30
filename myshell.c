/*
 * This code implements a simple shell program
 * It supports the internal shell command "exit", 
 * backgrounding processes with "&", input redirection
 * with "<" and output redirection with ">".
 * However, this is not complete.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

extern char **getaline();

#define EZMALLOC(type, n) \
  (type *) malloc((n) * sizeof(type))

/*
 * Handle exit signals from child processes
 */
void sig_handler(int signal) {
  int status;
  int result = wait(&status);

  printf("Wait returned %d\n", result);
}

/*
 * The main shell function
 */ 
main() {
  int i;
  char **args; // 2D char array of args
  int result;
  int block; // if the command does not end in an '&'
  int output; // whether output should be redirected
  int input; // whether there is redirected input
  char *output_filename; // name of file for output redirection
  char *input_filename; // name of input file

  // Set up the signal handler
  sigset(SIGCHLD, sig_handler); // SIGCHLD is sent when child process ends

  // Loop forever
  while(1) {

    // Print out the prompt and get the input
    printf(">");
    args = getaline();

    // No input, continue
    if(args[0] == NULL)
      continue;

    // Check for internal shell commands, such as exit
    if(internal_command(args))
      continue;

    // Handle piping
    int pipe = is_pipe(args);
    if (pipe) {
      // call pipe forking function - it handles everything from here
        fork_pipes(pipe+1, args);
    } else {

      // Check for an ampersand
      block = (ampersand(args) == 0); // block if not a background process

      // Check for redirected input
      input = redirect_input(args, &input_filename);

      switch(input) {
      case -1:
        printf("Syntax error!\n");
        continue;
        break;
      case 0:
        break;
      case 1:
        printf("Redirecting input from: %s\n", input_filename);
        break;
      }

      // Check for redirected output
      output = redirect_output(args, &output_filename);

      switch(output) {
      case -1:
        printf("Syntax error!\n");
        continue;
        break;
      case 0:
        break;
      case 1:
        printf("Redirecting output to: %s\n", output_filename);
        break;
      case 2:
        printf("Appending output to: %s\n", output_filename);
        break;
      }
      
      // Do the command
        do_command(args, block, 
            input, input_filename, 
            output, output_filename,
            0,0,0);
    }
  }
}

/*
 * Check for ampersand as the last argument
 */
int ampersand(char **args) {
  int i;

  for(i = 1; args[i] != NULL; i++) ;

  if(args[i-1][0] == '&') {
    free(args[i-1]);
    args[i-1] = NULL;
    return 1;
  } else {
    return 0;
  }
  
  return 0;
}

/* 
 * Check for internal commands
 * Returns true if there is more to do, false otherwise 
 */
int internal_command(char **args) {
  if(strcmp(args[0], "exit") == 0) {
    exit(0);
  }

  return 0;
}

/* 
 * Check for a pipe character
 * return number of pipes
 */
int is_pipe(char **args) {
  int i;
  int num_pipes = 0; // number of pipe characters, one less than number of commands
  for(i = 0; args[i] != NULL; i++) {
    if(args[i][0] == '|') {
      num_pipes = num_pipes+1;
    }
  }
  printf("Number of pipes: %d\n", num_pipes);
  return num_pipes;
}


/*
 * get first command from args with pipe, and cut off the pipe
 * to_exec points to remaining commands
 */
char **get_pipe_command(char **to_exec, int *num_args) {
  // printf("to_exec"); print_args(to_exec);
  int i = 0;
  char **first_args = EZMALLOC(char *, 20); // allocate char** for first command and its args
  while ((*to_exec != NULL) && (**to_exec != '|')) {
    printf("get_pipe_command i: %d\n", i);
    printf("%s\n", *to_exec);
    first_args[i] = *to_exec;
    to_exec++;
    i++;
  }
  *num_args = i;
  to_exec++;
  first_args[i] = NULL;
  printf("pipe command: ");
  print_args(first_args);
  printf("rest:");
  print_args(to_exec);
  return first_args;
}


int fork_pipes (int n, char **args) { // Function based on code from http://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
  int i;
  int in, fd[2]; // for piping
  // char **to_exec = args; // second pointer to args that always points to next command in args
  int block; // if the command does not end in an '&'
  int output; // whether output should be redirected
  int input; // whether there is redirected input
  char *output_filename; // name of file for output redirection
  char *input_filename; // name of input file
  int num_args = 0; // number of strings in first_args

  /* The first process should get its input from the original file descriptor 0.  */
  in = 0;

  /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  for (i = 0; i < n - 1; ++i) {
    printf("fork_pipes loop i: %d\n", i);
    pipe (fd);

    /* f[1] is the write end of the pipe, we carry `in` from the prev iteration.  */
    char **first_args = get_pipe_command(args, &num_args);
    args += num_args+1;
    printf("current command in fork: ");
    print_args(first_args);
    printf("to_exec in fork: ");
    print_args(args);

    // Check for an ampersand
    block = (ampersand(first_args) == 0); // block if not a background process

    // Check for redirected input
    input = redirect_input(first_args, &input_filename);

    // Check for redirected output
    output = redirect_output(first_args, &output_filename);

    // spawn_proc (in, fd[1], first_args);
    do_command(first_args, block, 
              input, input_filename, 
              output, output_filename,
              in, fd[1], 1);

    /* No need for the write and of the pipe, the child will write here.  */
    close (fd[1]);

    /* Keep the read end of the pipe, the next child will read from there.  */
    in = fd[0];

    int j=0;
    for (j = 0; j < num_args; j++) {
      free(first_args[j]);
    }
    free(first_args);
  }

  /* Last stage of the pipeline - set stdin be the read end of the previous pipe
     and output to the original file descriptor 1. */  
  if (in != 0)
    dup2 (in, 0);

  printf("****");
  print_args(args);
  /* Execute the last stage with the current process. */
  // Check for an ampersand
  block = (ampersand(args) == 0); // block if not a background process

  // Check for redirected input
  input = redirect_input(args, &input_filename);

  // Check for redirected output
  output = redirect_output(args, &output_filename);

  return do_command(args, block, 
                input, input_filename, 
                output, output_filename,
                0,0,0);
}



// /*
//   * method to handle strings of piped commands, calls do_pip_recursive
//   */
// int do_pipe(char **args) {

// }

//  /*
//   * recursive method to handle strings of piped commands
//   */
// int do_pipe_recursive(char **args, ) {
//   int output; // whether output should be redirected
//   int input; // whether there is redirected input
//   char *output_filename; // name of file for output redirection
//   char *input_filename; // name of input file

//   if(is_pipe(args)) {
//     // grab what's before the first pipe
//     char **first_args = get_pipe_command(args);
//     printf("doing pipe with: ");
//     print_args(first_args);

//     // Check for redirected input
//     input = redirect_input(first_args, &input_filename);

//     // Check for redirected output
//     output = redirect_output(first_args, &output_filename);

//     // redirect stdout to pipe

//     // run that command
//     do_command(first_args, block, 
//             input, input_filename, 
//             output, output_filename);
    
//     // recursively call this method with rest of args


//   // if there's another command
//   } else {
//     // else, put this commands's output in stdout

//   }
//   return 0;
// }

/* 
 * Do the command
 */
int do_command(char **args, int block,
	       int input, char *input_filename,
	       int output, char *output_filename,
         int in, int out, int is_pipe) {
  
  int result;
  pid_t child_id;
  int status;

  // Fork the child process
  child_id = fork();

  // Check for errors in fork()
  switch(child_id) {
  case EAGAIN:
    perror("Error EAGAIN: ");
    return;
  case ENOMEM:
    perror("Error ENOMEM: ");
    return;
  }

  if(child_id == 0) {

    // If we're coming from a pipe, handle I/O from in and out
    if(is_pipe) {
      printf("Running command from pipe\n");
      if (in != 0){
        dup2 (in, 0);
        close (in);
      }

      if (out != 1) {
        dup2 (out, 1);
        close (out);
      }
    // Otherwise, handle normally
    } else {

      // file redirection both in and out
      print_args(args);
      if (input && (output >= 1)) { 
        freopen(input_filename, "r", stdin);
        if(output == 1) { // re-write file, or create
          printf("input and output\n");
          freopen(output_filename, "w+", stdout);
        } else { // append file, or create
          printf("input and output append\n");
          freopen(output_filename, "a", stdout);
        }
      }
      // command with file redirection coming in
      else if (input) { 
        printf("input\n");
        freopen(input_filename, "r", stdin);
      }
      // command with output redirected to file
      else if (output >= 1) { 
        if(output == 1) { // re-write file, or create
          printf("output\n");
          freopen(output_filename, "w+", stdout);
        } else { // append file, or create
          printf("output append\n");
          freopen(output_filename, "a", stdout);
        }
      }
      // command with no arguments
      else if (args[1] == NULL) { 
        printf("no args\n");
      } 
      // command with arguments
      else {
        printf("args\n");
      }

    }
    // execute the actual command
    execvp(args[0], args);

    exit(-1);
  }

  // Wait for the child process to complete, if necessary
  if(block) {
    printf("Waiting for child, pid = %d\n", child_id);
    result = waitpid(child_id, &status, 0);
    if (result == -1) {
      printf("There was an error executing the process: %s\n", args[0]);
      return(1);
    }
  }

  return(0);
}

/*
 * Check for input redirection
 */
int redirect_input(char **args, char **input_filename) {
  int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {

    // Look for the <
    if(args[i][0] == '<') {
      free(args[i]);

      // Read the filename
      if(args[i+1] != NULL) {
        *input_filename = args[i+1];
      } else {
        return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
        args[j] = args[j+2];
      }

      return 1;
    }
  }

  return 0;
}

/*
 * Check for output redirection
 */
int redirect_output(char **args, char **output_filename) {
  int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {
    // Look for the >>
    if((args[i][0] == '>') && (args[i+1][0] == '>')) {
      free(args[i]);
      free(args[i+1]);

      // Get the filename 
      if(args[i+2] != NULL) {
        *output_filename = args[i+2];
      } else {
        return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
        args[j] = args[j+3];
      }

      return 2;
    }
    // else, look for the >
    else if(args[i][0] == '>') {
      free(args[i]);

      // Get the filename 
      if(args[i+1] != NULL) {
        *output_filename = args[i+1];
      } else {
        return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
        args[j] = args[j+2];
      }

      return 1;
    }
  }

  return 0;
}

// for testing purposes
int print_args(char **args) {
  int i;
  for(i=0; args[i] != NULL; i++) {
    printf("%s,", args[i]);
  }
  printf("\n");
  return 0;
}


