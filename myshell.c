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

    // Handle piping
    if (is_pipe(args)) {
      // call recursive pipe function
      do_pipe(args);
    } else {
      // Do the command
        do_command(args, block, 
            input, input_filename, 
            output, output_filename);
    }

    

    // free(args);
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
 */
int is_pipe(char **args) {
  int i;
  for(i = 0; args[i] != NULL; i++) {
    if(args[i][0] == '|') {
      return 1;
    }
  }
  return 0;
}

int do_pipe(char **args) {
  // grab what's before the first pipe

  // run that command

  // if there's another command
    // put this command's output in stdin

    // recursively call this method with rest of args

  // else, put this commands's output in stdout

  return 0;
}

/* 
 * Do the command
 */
int do_command(char **args, int block,
	       int input, char *input_filename,
	       int output, char *output_filename) {
  
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

    // Execute the command

    // file redirection both in and out
    printf("output:%d\n", output);
    if (input && (output >= 1)) { 
      freopen(input_filename, "r", stdin);
      if(output == 1) { // re-write file, or create
        printf("input and output\n");
        freopen(output_filename, "w+", stdout);
      } else { // append file, or create
        printf("input and output append\n");
        freopen(output_filename, "a", stdout);
      }
      execvp(args[0], args);
    }
    // command with file redirection coming in
    else if (input) { 
      printf("input\n");
      freopen(input_filename, "r", stdin);
      execvp(args[0], args);
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
      execvp(args[0], args);
    }
    // command with no arguments
    else if (args[1] == NULL) { 
      printf("no args\n");
      execvp(args[0], args);
    } 
    // command with arguments
    else if (!input && !output) {
      printf("args\n");
      execvp(args[0], args);
    }

    exit(-1);
  }

  // Wait for the child process to complete, if necessary
  if(block) {
    printf("Waiting for child, pid = %d\n", child_id);
    result = waitpid(child_id, &status, 0);
    printf("Waiting for child complete");
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


