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
		cplex.setParam(IloCplex::Param::TimeLimit, 8*60*60);
		cplex.setParam(IloCplex::Param::MIP::Limits::TreeMemory, 10);

		// Variável de decisão
		vector<IloIntVar*> y(phi * (n + 1), nullptr);
		vector<IloIntVar*> s(phi * (n + 1), nullptr);

		vector<vector<IloIntVar*>> x(phi*(n + 1), vector<IloIntVar*>(phi * (n+1), nullptr));
		vector<vector<IloNumVar*>> z(phi*(n + 1), vector<IloNumVar*>(phi * (n+1), nullptr));

		IloNum tol = cplex.getParam(IloCplex::EpInt);

		//Funcao obj
		IloExpr sum(env);
		for(int v = 0; v < phi * (n + 1); v++) {
			if(y[v] == nullptr) y[v] = new IloIntVar(env, 0, 1);
			if(s[v] == nullptr) s[v] = new IloIntVar(env, 0, 1);
			sum += p[v / phi] * (sigma * (*y[v]) + (1.0 - sigma) * (*s[v]));
		}
		//x[i][0] forall i
		//x[n+1][i] forall i
		
		TOP.add(IloMaximize(env, sum));

		{
			IloExpr sum(env);
			for(int i = phi; i < phi * (n + 1); i++) {
				if(s[i] == nullptr) s[i] = new IloIntVar(env, 0, 1);
				sum += *s[i];
			}
			TOP.add(sum <= NUM_PARADAS);
		}

		//(2)
		
		{
			IloExpr sum1(env), sum2(env);
			for(int j = phi; j < phi * (n + 1); j++) {
				if(x[0][j] == nullptr) x[0][j] = new IloIntVar(env, 0, 1);
				sum1 += *(x[0][j]);
			}

			for(int i = 0; i < phi * n; i++) {
				if(x[i][phi*n] == nullptr) x[i][phi*n] = new IloIntVar(env, 0, 1);
				sum2 += *(x[i][phi*n]);
			}
			
			TOP.add(sum1 == m);
			TOP.add(sum2 == m);
		}
		
		// EXPLICAR ESSA ADICAO QUE FIZ
		{
			IloExpr sum1(env), sum2(env);
			for(int i = 0; i < phi * (n + 1); i++) {

				if(x[i][0] == nullptr) x[i][0] = new IloIntVar(env, 0, 1);
				sum1 += *(x[i][0]);

				if(x[phi*n][i] == nullptr) x[phi*n][i] = new IloIntVar(env, 0, 1);
				sum2 += *(x[phi*n][i]);
			}
			TOP.add(sum1 == 0);
			TOP.add(sum2 == 0);

			for(int i = 1; i < phi; i++) {
				for(int j = 0; j < phi * (n + 1); j++) {
					if(x[j][i] == nullptr) x[j][i] = new IloIntVar(env, 0, 1);
					TOP.add(*(x[j][i]) == 0);
					
					if(x[i][j] == nullptr) x[i][j] = new IloIntVar(env, 0, 1);
					TOP.add(*(x[i][j]) == 0);

					if(x[phi*n + i][j] == nullptr) x[phi*n + i][j] = new IloIntVar(env, 0, 1);
					TOP.add(*(x[phi* n + i][j]) == 0);

					if(x[j][phi*n + i] == nullptr) x[j][phi * n + i] = new IloIntVar(env, 0, 1);
					TOP.add(*(x[j][phi* n + i]) == 0);
				}

				if(y[i] == nullptr) y[i] = new IloIntVar(env, 0, 1);
				TOP.add(*y[i] == 0);
				if(y[phi * n + i] == nullptr) y[i] = new IloIntVar(env, 0, 1);
				TOP.add(*y[phi * n + i] == 0);
			}
				
		}


		//(3)
		for(int i = phi; i < phi * n; i++) {
			IloExpr sum1(env), sum2(env);
			for(int j = 0; j < phi * (n + 1); j++) {
				if(i/phi != j/phi) {
					if(x[j][i] == nullptr) x[j][i] = new IloIntVar(env, 0, 1);
					sum1 += *(x[j][i]);

					if(x[i][j] == nullptr) x[i][j] = new IloIntVar(env, 0, 1);
					sum2 += *(x[i][j]);
				}
			}
			if(y[i] == nullptr) y[i] = new IloIntVar(env, 0, 1);
			TOP.add(sum1 == *y[i]);
			TOP.add(sum2 == *y[i]);
		}

		//(4)

		for(int j = phi; j < phi * n; j++) {
			if(z[0][j] == nullptr) z[0][j] = new IloNumVar(env, 0, L);
			TOP.add(*(z[0][j]) == dist[0][j / phi] * *(x[0][j]));
		}

		
		//(5)

		for(int i = phi; i < phi * n; i++) {
			IloExpr sum1(env), sum2(env), sum3(env);
			for(int j = 0; j < phi * (n + 1); j++) {
				if(i/phi != j/phi) {

					if(z[i][j] == nullptr) z[i][j] = new IloNumVar(env, 0, L);
					sum1 += *(z[i][j]);
					if(z[j][i] == nullptr) z[j][i] = new IloNumVar(env, 0, L);
					sum2 += *(z[j][i]);
					sum3 += dist[i/phi][j/phi] * *(x[i][j]);
				}
			}
			TOP.add(sum1 - sum2 == sum3 + t_parada * (*s[i]));
		}

		//(6)
		for(int i = 0; i < phi * (n + 1); i++) {
			for(int j = 0; j < phi * (n+1); j++) {
				if(i/phi != j/phi || (i/phi != 0 && j/phi != n)) {
					if(z[i][j] == nullptr) z[i][j] = new IloNumVar(env, 0, L);
					if(x[i][j] == nullptr) x[i][j] = new IloIntVar(env, 0, 1);
					TOP.add(*(z[i][j]) <= (L - dist[j/phi][n])* *(x[i][j]));
				}
			}
		}

		//(7)
		for(int i = 0; i < phi * (n+1); i++) {
			for(int j = 0; j < phi * (n + 1); j++) {
				if(i/phi != j/phi || (i != 0 && j/phi != n)) {
					if(z[i][j] == nullptr) z[i][j] = new IloNumVar(env, 0, L);
					if(x[i][j] == nullptr) x[i][j] = new IloIntVar(env, 0, 1);
					TOP.add(*(z[i][j]) >= (dist[0][i/phi] + dist[i/phi][j/phi]) * *(x[i][j]) );
				}
			}
		}

		// tprot
		for(int v = 1; v < n; v++) {
			for(int idx = 1; idx < phi; idx++) {

				if(y[phi*v + idx] == nullptr) y[phi*v + idx] = new IloIntVar(env, 0, 1);
				if(y[phi*v + idx - 1] == nullptr) y[phi*v + idx - 1] = new IloIntVar(env, 0, 1);
				TOP.add(*y[phi*v + idx] <= *y[phi*v + idx - 1]);
			}
		}

		double M = L + T_prot;

		for(int v = 1; v < n; v++) {
			for(int idx = 1; idx < phi; idx++) {
				IloExpr sum1(env), sum2(env);
				for(int u = 0; u < phi * (n + 1); u++) {
					if(u / phi != v) {
						if(z[u][phi*v + idx] == nullptr) z[u][phi*v + idx] = new IloNumVar(env, 0, L);
						sum1 += *(z[u][phi*v + idx]);
						if(z[u][phi*v + idx - 1] == nullptr) z[u][phi*v + idx - 1] = new IloNumVar(env, 0, L);
						sum2 += *(z[u][phi*v + idx - 1]);
					}
				}
				if(y[phi*v + idx] == nullptr) y[phi*v + idx] = new IloIntVar(env, 0, 1);
				TOP.add(sum1 + M*(1 - *y[phi*v + idx]) >= sum2 + T_prot);
			}
		}

		for(int i = phi; i < phi * (n + 1); i++) {
			if(y[i] == nullptr) y[i] = new IloIntVar(env, 0, 1);
			if(s[i] == nullptr) s[i] = new IloIntVar(env, 0, 1);
			TOP.add(*s[i] <= *y[i]);
		}

		//restricao adicional
		{
			IloExpr sum(env);
			for(int i = 0; i < phi * (n + 1); i++) {
				for(int j = 0; j < phi * (n + 1); j++) {
					if(i / phi != 0 || j / phi != n)	{
						if(x[i][j] == nullptr) x[i][j] = new IloIntVar(env, 0, 1);
						sum += (dist[i/phi][j/phi] )* (*x[i][j]); //ta faltando contar o tempo de parada
					}
				}
			}
			/*
			IloExpr sum2(env);
			for(int i = phi; i < phi * (n + 1); i++) {
				if(s[i] == nullptr) s[i] = new IloIntVar(env, 0, 1);
				sum2 += (*s[i]) * t_parada;
			}
			*/
			TOP.add(sum <= m*L);
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
				if(cplex.getValue(*(x[0][i])) >= 1.0 - tol) {
					vector<int> cnt_visitas(n + 1);
					double len_caminho = dist[0][i / phi];
					cout << "viatura " << cnt++ << ": " << endl;
					int curr = i;
					double visit_time = cplex.getValue(*(z[0][curr]));
					tabela_visita[curr / phi].push_back(visit_time);
					cout << "vertice,tempo_visita" << endl << endl;
					cout << "base," << 0 << endl;
					while(curr != phi*n) {

						cnt_visitas[curr / phi]++;
						
						for(int next = 0; next < phi * (n + 1); next++) {
							if(curr / phi == next / phi) continue;
							if(cplex.getValue(*x[curr][next]) >= 1.0 - tol) {
								int parou_passou = (cplex.getValue(*s[curr]) >= 1.0 - tol);
								cout << curr/phi + 1 << "," << visit_time << "," << parou_passou << endl;
								visit_time = cplex.getValue(*(z[curr][next]));
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
				for(int j = 0; j < (int) tabela_visita[i].size(); j++) {
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
						if(cplex.getValue(*x[i][j]) >= 1.0 - tol) g_sol[i].push_back(j);
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
			if(cplex.getValue(*s[i] )>= 1.0 - tol) parou[i / phi]++;
			else if(cplex.getValue(*y[i]) >= 1.0 - tol) passou[i / phi]++;
		}

		for(int i = 0; i < n; i++) cout << parou[i] << " ";
		cout << endl;
		for(int i = 0; i < n; i++) cout << passou[i] << " ";
		cout << endl;


		cout << (OK ? "certo" : "errado" ) << endl;
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
