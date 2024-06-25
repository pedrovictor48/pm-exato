#include <ilcplex/ilocplex.h>
#include <vector>
#include <algorithm> 

using namespace std;

bool dfs(int st, int curr, vector<vector<int>> &g, vector<int> &vis) {
	if(curr == st) return false;
	vis[curr] = 1;
	bool ret = true;
	for(int next : g[curr]) {
		if(!vis[next]) {
			ret = ret && dfs(st, next, g, vis);
		}
			
	}
	return ret;
}

int main(int argc, char* argv[])
{
	bool OK = true;
    int m;
    double L;

    

	// Criando o ambiente
	IloEnv env;
	try
	{

		// Entrada
		int n; cin >> n;
		int m; cin >> m;
		double L; cin >> L;
		double T_prot; cin >> T_prot;
		int phi = floor(L / T_prot);
		phi = 3;
		double t_parada = 1000;
		double sigma = 1.0/3;
		const int NUM_PARADAS = n*phi;

		vector<int> p(n + 1);
		for(int i = 0; i < n; i++) {
			int v; cin >> v;
			cin >> p[v - 1];
		}
		p[n] = p[0];

		vector<vector<double>> dist(n + 1, vector<double>(n + 1));
		for(int i = 0; i < n; i++) {
			for(int j = 0; j < n; j++) cin >> dist[i][j];
		}
		// ja que 0 == n	
		for(int i = 0; i < n; i++) {
			dist[n][i] = dist[0][i];
			dist[i][n] = dist[i][0];
		}
		dist[n][n] = 0;

		
		
		// Modelo
		IloModel TOP(env, "Team OP");

		// Inicializando o objeto cplex
		IloCplex cplex(TOP);
		//cplex.setOut(env.getNullStream());
		//cplex.setParam(IloCplex::Param::TimeLimit, 300);

		// Variável de decisão
		IloIntVarArray y(env, phi*(n + 1), 0, 1);

		
		IloArray<IloIntVarArray> x(env, phi * (n + 1));
		for(int i = 0; i < phi * (n+1); i++) {
			x[i] = IloIntVarArray(env, phi * (n+1), 0, 1);
		}


		IloIntVarArray s(env, phi*(n + 1), 0, 1);
		

		//vector<vetor<IloIntVar*>> map(n, vector<IloIntVar*>(n, nullptr));

		//TOP.add(*map[i][j] == 0);
		
			

		IloArray<IloNumVarArray> z(env, phi * (n+1));
		for(int i = 0; i < phi * (n + 1); i++) z[i] = IloNumVarArray(env, phi * (n+1), 0, L);

		IloNum tol = cplex.getParam(IloCplex::EpInt);

		//Funcao obj
		IloExpr sum(env);
		//for(int v = 0; v < phi * (n + 1); v++) sum += p[v / phi] * (sigma * y[v] + (1.0 - sigma) * s[v]);
		for(int v = 0; v < phi * (n + 1); v++) sum += p[v / phi] * (sigma * y[v] + (0) * s[v]);
		TOP.add(IloMaximize(env, sum));
		//TOP.add(s[2] == 1);

		{
			IloExpr sum(env);
			for(int i = phi; i < phi * (n + 1); i++) {
				sum += s[i];
			}
			//TOP.add(sum <= NUM_PARADAS);
		}

		//(2)
		
		{
			IloExpr sum1(env), sum2(env);
			for(int j = phi; j < phi * (n + 1); j++) 
				sum1 += x[0][j];

			for(int i = 0; i < phi * n; i++) sum2 += x[i][phi*n];

			TOP.add(sum1 == m);
			TOP.add(sum2 == m);
		}
		
		// EXPLICAR ESSA ADICAO QUE FIZ
		{

			IloExpr sum1(env), sum2(env);
			for(int i = 0; i < phi * (n + 1); i++) {
				sum1 += x[i][0];
				sum2 += x[phi*n][i];
			}
			TOP.add(sum1 == 0);
			TOP.add(sum2 == 0);

			for(int i = 1; i < phi; i++) {
				for(int j = 0; j < phi * (n + 1); j++) {
					TOP.add(x[j][i] == 0);
					TOP.add(x[i][j] == 0);

					TOP.add(x[phi* n + i][j] == 0);
					TOP.add(x[j][phi* n + i] == 0);
				}

				TOP.add(y[i] == 0);
				TOP.add(y[phi * n + i] == 0);
			}
				
		}


		//(3)
		for(int i = phi; i < phi * n; i++) {
			IloExpr sum1(env), sum2(env);
			for(int j = 0; j < phi * (n + 1); j++) {
				if(i/phi != j/phi) {
					sum1 += x[j][i];
					sum2 += x[i][j];
				}
			}
			TOP.add(sum1 == y[i]);
			TOP.add(sum2 == y[i]);
		}

		//(4)

		for(int j = phi; j < phi * n; j++) {
			TOP.add(z[0][j] == dist[0][j / phi] * x[0][j]);
		}

		
		//(5)

		for(int i = phi; i < phi * n; i++) {
			IloExpr sum1(env), sum2(env), sum3(env);
			for(int j = 0; j < phi * (n + 1); j++) {
				if(i/phi != j/phi) {
					sum1 += z[i][j];
					sum2 += z[j][i];
					sum3 += dist[i/phi][j/phi] * x[i][j];
				}
			}
			TOP.add(sum1 - sum2 == sum3 + t_parada * s[i]);
		}

		//(6)
		for(int i = 0; i < phi * (n + 1); i++) {
			for(int j = 0; j < phi * (n+1); j++) {
				if(i/phi != j/phi || (i/phi != 0 && j/phi != n)) {
					TOP.add(z[i][j] <= (L - dist[j/phi][n])*x[i][j]);
				}
			}
		}

		//(7)
		for(int i = 0; i < phi * (n+1); i++) {
			for(int j = 0; j < phi * (n + 1); j++) {
				if(i/phi != j/phi || (i != 0 && j/phi != n)) {
					TOP.add(z[i][j] >= (dist[0][i/phi] + dist[i/phi][j/phi]) * x[i][j] );
				}
			}
		}

		// tprot
		for(int v = 1; v < n; v++) {
			for(int idx = 1; idx < phi; idx++) {
				TOP.add(y[phi*v + idx] <= y[phi*v + idx - 1]);
			}
		}

		double M = L + T_prot;

		for(int v = 1; v < n; v++) {
			for(int idx = 1; idx < phi; idx++) {
				IloExpr sum1(env), sum2(env);
				for(int u = 0; u < phi * (n + 1); u++) {
					if(u / phi != v) {
						sum1 += z[u][phi*v + idx];
						sum2 += z[u][phi*v + idx - 1];
					}
				}
				TOP.add(sum1 + M*(1 - y[phi*v + idx]) >= sum2 + T_prot);
			}
			
		}

		for(int i = phi; i < phi * (n + 1); i++) {
			TOP.add(s[i] <= y[i]);
		}

		cplex.exportModel("modelo.lp");
		if ( cplex.solve() ) {
			cerr << "Premio ótimo: " << cplex.getObjValue() << endl;
		}
		cout << endl;
		{
			vector<vector<int>> tabela_visita(n + 1);
			int cnt = 1;
			for(int i = phi; i < phi * n; i++) {
				if(cplex.getValue(x[0][i]) >= 1.0 - tol) {
					vector<int> cnt_visitas(n + 1);
					double len_caminho = dist[0][i / phi];
					cout << "viatura " << cnt++ << ": " << endl;
					int curr = i;
					double visit_time = cplex.getValue(z[0][curr]);
					tabela_visita[curr / phi].push_back(visit_time);
					cout << "vertice,tempo_visita" << endl << endl;
					cout << "base," << 0 << endl;
					while(curr != phi*n) {

						cnt_visitas[curr / phi]++;
						
						for(int next = 0; next < phi * (n + 1); next++) {
							if(curr / phi == next / phi) continue;
							if(cplex.getValue(x[curr][next]) >= 1.0 - tol) {
								cout << curr/phi + 1 << "," << visit_time << endl;
								visit_time = cplex.getValue(z[curr][next]);
								tabela_visita[next / phi].push_back(visit_time);
								len_caminho += dist[curr / phi][next / phi];
								curr  = next;
								break;
							}
								
						}
					}
					cout << "base" << "," << visit_time << endl;
					cout << len_caminho << endl << endl;
					if(len_caminho > L) OK = false;
				}
			} 
			for(int i = 0; i < n; i++) {
				if(tabela_visita[i].empty()) continue;
				sort(tabela_visita[i].begin(), tabela_visita[i].end());
				cout << i << ": ";
				for(int j = 0; j < tabela_visita[i].size(); j++) {
					cout << tabela_visita[i][j] << ", ";
					if(j > 0) {
						double diff = tabela_visita[i][j] - tabela_visita[i][j - 1];
						if(diff < T_prot) OK = false;
					}
				}
				cout << endl;
			}

			{
				vector<vector<int>> g_sol(phi*(n + 1));
				vector<int> v_sol(phi*(n + 1));

				for(int i = 0; i < phi*(n + 1); i++) {
					for(int j = 0; j < phi*(n + 1); j++) {
						if(cplex.getValue(x[i][j]) >= 1.0 - tol) g_sol[i].push_back(j);
						v_sol[i] = v_sol[j] = 1;
					}
				}
				for(int st = 0; st < phi*(n + 1); st++) {
					if(!v_sol[st]) continue;
					for(int next : g_sol[st]) {
						vector<int> vis(phi*(n + 1));
						bool ret = dfs(st, next, g_sol, vis);
						if(!ret) OK = false;
					}
				}
			}

		}
		cout << endl;
		vector<int> parou(n), passou(n);
		for(int i = phi; i < phi * (n + 1); i++) {
			if(cplex.getValue(s[i] )>= 1.0 - tol) parou[i / phi]++;
			else if(cplex.getValue(y[i]) >= 1.0 - tol) passou[i / phi]++;
		}

		for(int i = 0; i < n; i++) cout << parou[i] << " ";
		cout << endl;
		for(int i = 0; i < n; i++) cout << passou[i] << " ";
		cout << endl;


		cout << (OK ? "certo" : "errado" ) << endl;

		/*
		for(int i = phi; i < phi*(n + 1); i++) {
			cout << i/phi << ": " << cplex.getValue(x[0][i]) << " ";
		}
		cout << endl;
		*/

}
	catch (const IloException& e)
	{
		cerr << "Exception caught: " << e << endl;
	}
	catch (...)
	{
		cerr << "Unknown exception caught!" << endl;
	}

	env.end();
	return 0;
}