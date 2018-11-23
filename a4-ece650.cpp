// Include files
#include <iostream>
#include <vector>
#include <string>
#include <algorithm> 
#include "minisat/core/SolverTypes.h"
#include "minisat/core/Solver.h"
//#define DEBUG_MODE // enable debug prints

using namespace std;
//Enums Used
enum Result 
{
    NOT_FOUND,
    FOUND
};
enum STATUS 
{
    WAITING_FOR_VERTEX,
    WAITING_FOR_EDGES,
    GO_CALCULATE,
    DONE_CALCULATING,
};

// Function declarations
void Vertex_Cover_Printer(vector<int> *vertex_cover);   
void init_edge_vector(int vertex_num, int old_vertex_num);
void command_parser(string input);
void edge_command(string input, size_t space);
void vertex_command(string input, size_t space);
int edge_number_calc(string input, size_t space);
string trim(const string& str);

// Gloabl Variables
int vertex_num = 0; //i to n
int k_solver = 0; // j to k
int edge_number = 0; // Captured edges
vector<int> vertex_cover;  
int **edges = NULL;
int status_flag = WAITING_FOR_VERTEX;

// Main //
int main() 
{   
    using Minisat::Var;
    using Minisat::mkLit;
    using Minisat::lbool;
    bool exit_flag = false;
    int last_result = NOT_FOUND;
    int last_k=vertex_num;
    int upper_bound = vertex_num-1;
    int lower_bound = 0;
    string input;
    while (std::getline(cin, input))
    {
      input=trim(input);
      command_parser(input);
      input = "";
    
        if(status_flag == GO_CALCULATE)
        {
            status_flag = DONE_CALCULATING;
            k_solver = vertex_num/2; // j to k
            while(exit_flag == false)
            {   
                // Exit search routine
                if(last_k==k_solver)
                {
                    if(last_result == NOT_FOUND){k_solver+=1;exit_flag = true;}
                    else if(last_result == FOUND){exit_flag = true;}
                    #ifdef DEBUG_MODE
                    std::cout<<"LAST K: "<<k_solver<<endl;
                    #endif
                }
                last_k=k_solver;

                Minisat::Solver solver;
                Minisat::Var Matrix[vertex_num][k_solver];

                // Init variables for MiniSAT
                for (int n = 0; n < vertex_num; ++n) 
                {
                    for (int i = 0; i < k_solver; ++i) 
                    {
                        Matrix[n][i] = solver.newVar();
                    }
                }

                // Clause 1
                for(int k=0;k<k_solver;k++)
                {
                    Minisat::vec<Minisat::Lit> literals;
                    for(int i=0;i<vertex_num;i++)
                    {
                        literals.push(Minisat::mkLit(Matrix[i][k]));
                        #ifdef DEBUG_MODE
                        cout<<"X"<<"["<<i<<","<<k<<"]"<<" V ";
                        #endif
                    }
                    solver.addClause(literals);
                    literals.clear();
                    #ifdef DEBUG_MODE
                    cout<<endl;
                    #endif 
                }

                #ifdef DEBUG_MODE
                cout<<endl;
                #endif

                // Clause 2
                for(int i=0;i<vertex_num;i++)
                {    
                    for(int k=0;k<k_solver-1;k++)
                    {   Minisat::vec<Minisat::Lit> literals;
                        literals.push(~Minisat::mkLit(Matrix[i][k]));
                        literals.push(~Minisat::mkLit(Matrix[i][k+1]));
                        solver.addClause(literals);
                        literals.clear();
                        #ifdef DEBUG_MODE
                        cout<<"~X"<<"["<<i<<","<<k<<"]"<<" V "<<"~X"<<"["<<i<<","<<k+1<<"]"<<" V "<<endl;
                        #endif
                    }
                }

                #ifdef DEBUG_MODE
                cout<<endl;
                #endif

                // Clause 3
                for(int k=0;k<k_solver;k++)
                {
                    for(int i=0;i<vertex_num-1;i++)
                    {
                        for(int j=i+1;j<vertex_num;j++)
                        {
                        Minisat::vec<Minisat::Lit> literals;
                        literals.push(~Minisat::mkLit(Matrix[i][k]));
                        literals.push(~Minisat::mkLit(Matrix[j][k]));
                        solver.addClause(literals);
                        literals.clear();
                        #ifdef DEBUG_MODE
                        cout<<"~X"<<"["<<i<<","<<k<<"]"<<" V "<<"~X"<<"["<<j<<","<<k<<"]"<<" V "<<endl;
                        #endif
                        }
                    }
                } 

                #ifdef DEBUG_MODE
                cout<<endl;
                #endif

                // Clause 4
                for (int i = 0; i < edge_number; i++) 
                { 
                    Minisat::vec<Minisat::Lit> literals;
                    for (int j = 0; j < 2; j++)
                    {
                        for(int k=0;k<k_solver;k++)
                        {
                        literals.push(Minisat::mkLit(Matrix[edges[i][j]][k]));
                        #ifdef DEBUG_MODE
                        cout<<"X"<<"["<<edges[i][j]<<","<<k<<"]"<<" V ";
                        #endif
                        }
                    }
                    solver.addClause(literals);
                    literals.clear();
                    #ifdef DEBUG_MODE
                    cout<<endl;
                    #endif
                } 

               // Check for solution and retrieve model if found
               auto sat = solver.solve();
               // Solution found
               if (sat) 
               {
                  vertex_cover.clear();
                    for (int n = 0; n < vertex_num; ++n) 
                    {
                        for (int i = 0; i < k_solver; ++i) 
                        {
                            if (solver.modelValue(Matrix[n][i]) == Minisat::l_True) 
                            {   

                                vertex_cover.push_back(n);
                                #ifdef DEBUG_MODE
                                std::cout<<"n: "<<n<<endl;    
                                std::cout<<"i: "<<i<<endl; 
                                #endif
                            }
                        }
                    }
                    last_result = FOUND;

                    upper_bound = k_solver-1;
               }
               // Solution not found
               else 
               {  last_result = NOT_FOUND;
                  lower_bound = k_solver+1;
               }
               k_solver=(lower_bound+upper_bound)/2;  
            }
            Vertex_Cover_Printer(&vertex_cover);
               
            delete [] *edges;
            delete [] edges;
        }   
    }
    return 0;
}


// Functions definitions
void Vertex_Cover_Printer(vector<int> *v)
{   
    vector<int> vertex_cover=*v;
    std::sort(vertex_cover.begin(), vertex_cover.end());
    for (int n = 0; n < vertex_cover.size(); ++n) 
    { 
        cout<< vertex_cover[n]<<" ";
    }
    cout << endl;
}

void command_parser(string input)
   {
      std::size_t vertex_ = input.find("V");
      std::size_t edge_ = input.find("E");
      std::size_t space = input.find(" ");
      
      // VERTEX COMMAND
      if ((vertex_ != -1) && ((space != -1))) { vertex_command(input, space); }
      //EDGE COMMAND
      else if ((edge_  != -1) && ((space != -1))) { edge_command(input, space); }
      //INVALID COMMAND
      else {std::cerr << " Error: Command not recognized \n";}
   }

void vertex_command(string input, size_t space)
   {
      
      input = input.substr(space, input.length() - 1); // Parse command from line
     vertex_num = atoi(input.c_str());// Convert parsed data
      if (vertex_num > 1)
      { 

         status_flag = WAITING_FOR_EDGES;
      }
      else
      {
         status_flag = WAITING_FOR_VERTEX;
      }

   }
   
void init_edge_vector(int vertex_num, int old_vertex_num)
   {

            
        edges = new int *[vertex_num] ;
        for( int i = 0 ; i < vertex_num ; i++ )
        {
            edges[i] = new int[2];
        }
      
   }      
      
void edge_command(string input, size_t space)
{
    if (status_flag == WAITING_FOR_EDGES) {
       int old_edge_number=edge_number;
       edge_number=edge_number_calc( input,  space);
       init_edge_vector(edge_number,old_edge_number);// Assign vertex num
       std::string s = input.substr(space, input.length() - 1); // Remove parsed command from line
       std::string delimiter = "}"; // delimiter of end of line
       size_t pos;
       std::string token;
       std::string token2;
       int i;
       while ((pos = s.find(delimiter)) != std::string::npos) // iterate through line to find data
       {
          size_t x = s.find(',') - 1 - s.find('<');
          size_t y = s.find('>') - 1 - s.find(',');
          token = s.substr(s.find('<') + 1, x);
          token2 = s.substr(s.find(',') + 1, y);
          int var1 = atoi(token.c_str());
          int var2 = atoi(token2.c_str());
          edges[i][0]=var1;
          edges[i][1]=var2;
          s.erase(0, s.find_first_of(">") + 2);// erase parsed data for next iteration/
          status_flag = GO_CALCULATE;
          i++;
       }
    }
    else if (status_flag == WAITING_FOR_VERTEX) 
    {
       std::cerr << " Error: need to assign number of vertex first use command V \n";
    }
    else if (status_flag == GO_CALCULATE) 
    {
       std::cerr << " Error: Edge list already captured to enter new list first change the # of vertexes according to the new list \n";
    }
    else if (status_flag == DONE_CALCULATING) 
    {
       std::cerr << " Error: Calculation for the provided input has been made enter a new vertex number and graph \n";
    }
}
          
int edge_number_calc(string input, size_t space)
{
   int i=0;
   std::string s = input.substr(space, input.length() - 1); // Remove parsed command from line
   string delimiter = "}"; // delimiter of end of line
   size_t pos;
   while ((pos = s.find(delimiter)) != std::string::npos) // iterate through line to find # edges
   {
      s.erase(0, s.find_first_of(">") + 2);// erase edge/
      i++;
   }
   return i;
}


string trim(const string& str)
{
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

// END