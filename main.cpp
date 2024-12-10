#include <ilcplex/ilocplex.h>
#include <vector>
#include <algorithm>

using namespace std;

int transf(int v, int phi) {
    if(v == 0) return 0;
    return (v-1)/phi + 1;
}

int32_t main(int argc, char* argv[]) {
    string file_path = argv[1];

    IloEnv env;
    IloModel TOP(env, "Team OP");
    IloCplex CPLEX(TOP);

    // Adiciona o limite de tempo de 3 horas (10800 segundos)
    CPLEX.setParam(IloCplex::TiLim, 10800);

    // Adiciona o limite de RAM para 15 GB
    CPLEX.setParam(IloCplex::WorkMem, 10000);

    ifstream file(file_path);
    // entrada 
    int n; file >> n;
    int m; file >> m;
    double L; file >> L;
    //L = L / 60;
    double T_prot;
    file >> T_prot;
    double velocidade;
    file >> velocidade;

    int phi = floor((double) L / T_prot);
    
    double t_parada = 15;
    double sigma = 1.0/3;
    double tr_coef = 1/((velocidade/60)*1000);


    vector<int> p(n + 1);
    for(int i = 0; i < n; i++) {
        int v; file >> v;
        //v--;
        file >> p[v];
    }
    p[n] = p[0];

    vector<vector<double>> dist(n + 1, vector<double>(n + 1));
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            double distance;
            file >> distance;
            dist[i][j] = distance * tr_coef;
        }
    }
    // ja que 0 == n	
    for(int i = 0; i < n; i++) {
        dist[n][i] = dist[0][i];
        dist[i][n] = dist[i][0];
    }
    dist[n][n] = 0;

    n--;

    const int f = n*phi + 1;
    cout << "f: " << f << endl;
    vector<IloIntVar*> y(f+1, nullptr);
	vector<IloIntVar*> s(f+1, nullptr);
	vector<vector<IloIntVar*>> x(f+1, vector<IloIntVar*>(f+1, nullptr));
	vector<vector<IloNumVar*>> z(f+1, vector<IloNumVar*>(f+1, nullptr));

    //1
    {
        IloExpr sum(env);
        for(int v = 1; v <= f-1; v++) {
            if(y[v] == nullptr) y[v] = new IloIntVar(env, 0, 1);
            if(s[v] == nullptr) s[v] = new IloIntVar(env, 0, 1);
            sum += (sigma*(*y[v]) + (1 - sigma)*(*s[v]))*p[transf(v,phi)];
        }
        TOP.add(IloMaximize(env, sum));
    }



    
    //2
    {
        IloExpr sum1(env), sum2(env);
        for(int v = 1; v <= f-1; v++) {

            if(x[0][v] == nullptr) x[0][v] = new IloIntVar(env, 0, 1);
            if(x[v][f] == nullptr) x[v][f] = new IloIntVar(env, 0, 1);
            sum1 += (*x[0][v]);
            sum2 += (*x[v][f]);
        }
        TOP.add(sum1 == m);
        TOP.add(sum2 == m);
    }

    //3
    {
        for(int v = 1; v <= f-1; v++) {
            IloExpr sum1(env), sum2(env);
            //vizinhos neg
            for(int u = 0; u <= f-1; u++) {
                if(u == v) continue;
                if(x[u][v] == nullptr) x[u][v] = new IloIntVar(env, 0, 1);
                sum1 += (*x[u][v]);
            }

            //vizinhos pos
            for(int u = 1; u <= f; u++) {
                if(u == v) continue;
                if(x[v][u] == nullptr) x[v][u] = new IloIntVar(env, 0, 1);
                sum2 += (*x[v][u]);
            }

            if(y[v] == nullptr) y[v] = new IloIntVar(env, 0, 1);
            TOP.add(sum1 == (*y[v]));
            TOP.add(sum2 == (*y[v]));
        }
    }


    //4
    {
        for(int v = 1; v <= f-1; v++) {
            int v_transf = transf(v, phi);
            if(z[0][v] == nullptr) z[0][v] = new IloNumVar(env, 0, L);
            if(x[0][v] == nullptr) x[0][v] == new IloIntVar(env, 0, 1);
            TOP.add(*z[0][v] == dist[0][v_transf] * (*x[0][v]));
        }
    }

    //5
    {
        for(int v = 1; v <= f-1; v++) {
            IloExpr sum1(env), sum2(env), sum3(env);
            //vizinhos pos
            for(int u = 1; u <= f; u++) {
                int v_transf = transf(v, phi), u_transf = transf(u, phi);
                if(u == v) continue;
                if(z[v][u] == nullptr) z[v][u] = new IloNumVar(env, 0, L);
                if(x[v][u] == nullptr) x[v][u] = new IloIntVar(env, 0, 1);
                sum1 += (*z[v][u]);
                sum3 += (dist[v_transf][u_transf] * (*x[v][u]));
            }

            //vizinhos neg
            for(int u = 0; u <= f-1; u++) {
                if(u == v) continue;
                if(z[u][v] == nullptr) z[u][v] = new IloNumVar(env, 0, L);
                sum2 += (*z[u][v]);
            }
            if(s[v] == nullptr) s[v] = new IloIntVar(env, 0, 1);
            TOP.add(sum1 - sum2 == sum3 + t_parada*(*s[v]));
        }
    }


    //6
    {
        for(int u = 0; u <= f-1; u++) {
            for(int v = 1; v <= f; v++) {
                if(u == v) continue;
                if(u == 0 && v == f) continue;
                int v_transf = transf(v, phi), u_transf = transf(u, phi);
                if(z[u][v] == nullptr) z[u][v] = new IloNumVar(env, 0, L);
				if(x[u][v] == nullptr) x[u][v] = new IloIntVar(env, 0, 1);
                TOP.add(*(z[u][v]) <= (L - dist[v_transf][n])* *(x[u][v]));
            }
        }
    }

    //7
    {
        for(int v = 0; v <= f-1; v++) {
            for(int u = 1; u <= f; u++) {
                if(u == v) continue;
                if(v == 0 && u == f) continue;
                int v_transf = transf(v, phi), u_transf = transf(u, phi);
				if(z[v][u] == nullptr) z[v][u] = new IloNumVar(env, 0, L); 
                if(x[v][u] == nullptr) x[v][u] = new IloIntVar(env, 0, 1);
				TOP.add((*z[v][u]) >= (dist[0][v_transf] + dist[v_transf][u_transf]) * (*x[v][u]));
            }
        }
    }

    //8
    {
        //x[0][f] = new IloIntVar(env, 0, m);
    }

    //9
    {
        for(int v = 0; v <= f; v += phi) {
            for(int i = 1; i <= phi - 1 && v + i <= f; i++) {
                if(y[v + i] == nullptr) y[v + i] = new IloIntVar(env, 0, 1);
                if(y[v + i - 1] == nullptr) y[v + i - 1] = new IloIntVar(env, 0, 1);
                TOP.add(*y[v + i] <= *y[v + i - 1]);
            }
        }
    }

    //10
    {
        IloExpr sum1(env), sum2(env);
        for(int v = 0; v <= f; v += phi) {
            for(int i = 1; i <= phi - 1 && v + i <= f; i++) {
                IloExpr sum1(env), sum2(env);
                for(int u = 0; u <= n*phi; u++) {
                    if(u == v + i) continue;
                    if(z[u][v + i] == nullptr) z[u][v + i] = new IloNumVar(env, 0, L);
                    sum1 += *z[u][v + i];
                }
                for(int u = 0; u <= n*phi; u++) {
                    if(u == v + i - 1) continue;
                    if(z[u][v + i - 1] == nullptr) z[u][v + i - 1] = new IloNumVar(env, 0, L);
                    sum2 += *z[u][v + i - 1];
                }
                const int M = L + T_prot;
                if(y[v + i] == nullptr) y[v + i] = new IloIntVar(env, 0, 1);
                if(s[v + i - 1] == nullptr) s[v + i - 1] = new IloIntVar(env, 0, 1);
                TOP.add(sum1 + M*(1 - *y[v + i]) >= sum2 + T_prot + t_parada * (*s[v + i - 1]));
            }
        }
    }


    //11
    {
        for(int v = 1; v <= f-1; v++) {
            if(s[v] == nullptr) s[v] = new IloIntVar(env, 0, 1);
            if(y[v] == nullptr) y[v] = new IloIntVar(env, 0, 1);
            TOP.add(*s[v] <= *y[v]);
        }
    }

    //retricao adicional
    {
        IloExpr sum(env);
        for(int u = 0; u <= f - 1; u++) {
            for(int v = 1; v <= f; v++) {
                if(u == 0 && v == f) continue;
                if(u == v) continue;
                sum += dist[transf(u,phi)][transf(v,phi)] * (*x[u][v]);
            }
        }
        TOP.add(sum <= m*L);
    }

	IloNum tol = CPLEX.getParam(IloCplex::EpInt);
    cout << "a" << endl;

    
    CPLEX.exportModel("modelo.lp");
	if ( CPLEX.solve() ) {
		cerr << "Premio ótimo: " << CPLEX.getObjValue() << endl;
	}
    
    cout << endl << endl << endl; 
	int curr = 0, viatura = 1;
    vector<vector<double>> tempos_visita(n+1, vector<double>(0));
	for(int init = 1; init <= f-1; init++) {
		if(x[0][init] == nullptr) continue;
		if(CPLEX.getValue(*x[0][init]) >= 1.0 - tol) {
			cout << "rota " << viatura << ":" << endl << endl;
            cout << "\t";
            cout << "vertice\ttempo\tparou" << endl;
            cout << "\t";
			cout << "base" << "\t0\tnao" << endl;
            tempos_visita[0].push_back(0);
		    if(z[0][init] == nullptr) continue;
			double tempo = CPLEX.getValue(*z[0][init]);
			curr = init;
            cout << "\t";
			cout  << transf(init,phi) << "\t" << tempo << "\t";
            cout << (CPLEX.getValue(*s[init]) >= 1.0 - tol ? "sim" : "nao") << endl;
            tempos_visita[transf(init,phi)].push_back(tempo);
			while(curr != f) {
				for(int next = 1; next <= f; next++) {
                    if(next == curr) continue;
                    if(x[curr][next] == nullptr) continue;
					if(CPLEX.getValue(*x[curr][next]) >= 1.0 - tol) {
						double tempo = CPLEX.getValue(*z[curr][next]);
                        cout << "\t";
						if(next == f) cout << "base" << "\t" << tempo << "\t";
						else cout << transf(next,phi) << "\t" << tempo << "\t";
                        if(next != f) {
                            int parou = CPLEX.getValue(*s[next]);
                            cout << (parou >= 1.0 - tol ? "sim" : "nao") << endl;
                        }
                        else cout << "nao" << endl;
                        if(next != f) tempos_visita[transf(next,phi)].push_back(tempo);
                        else tempos_visita[0].push_back(tempo);
						curr = next;
						break;
					}
				}
			}
		    viatura++;
		}
	}
    cout << endl;
    cout << "vertice\t\ttempos visitado" << endl;
    for(int u = 0; u < n+1; u++) {
        cout << u << "\t\t";
        int vec_size = tempos_visita[u].size();
        for(int i = 0; i < vec_size; i++) {
            cout << tempos_visita[u][i];
            if(i < vec_size - 1) cout << ", ";
        }
        cout << endl;
    }

    cout << endl;
    double premio = CPLEX.getObjValue();
    cout << "premio:\t\t\t" << premio << endl;
    cout << "numero de vertices:\t" << n+1 << endl;
    cout << "tempo de protecao(min):\t" << T_prot << endl;
    cout << "velocidade(km/h):\t" << velocidade << endl;
    cout << "tempo de execucao:\t" << CPLEX.getCplexTime() << endl;
    cout << "status:\t\t\t" << CPLEX.getCplexStatus() << endl;
    cout << "GAP:\t\t\t" << CPLEX.getMIPRelativeGap() << endl;
}