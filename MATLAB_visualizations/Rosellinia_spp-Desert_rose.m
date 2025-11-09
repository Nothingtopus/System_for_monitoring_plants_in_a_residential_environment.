% Configurações do canal:
channelID = ---; % Número de identificação do canal.
readAPIKey = '---'; % Chave de leitura no canal, para aquisição de dados.

% Abaixo se realiza a leitura do último valor do field 2 (correspondente a temperatura do ambiente) 
% e a salva na variável temperatura:
temperatura = thingSpeakRead(channelID, 'Fields', 2, 'NumPoints', 1, 'ReadKey', readAPIKey);
% Abaixo se realiza as leituras dos últimos 20 dias do field 5 (correspondente a umidade do solo) 
% e a salva na variável dados:
atualmente = datetime('now');
vinte_dias = datetime('now') - days(20);

dados = thingSpeakRead(channelID, 'Fields', 5, 'DateRange', [vinte_dias atualmente], 'ReadKey', readAPIKey, 'OutputFormat', 'table');

% Estabelece a leitura de limite superior de solo úmido:
limite = 1200;
% Gera um vetor de "parametros" binários que correspondem a 1 caso a leitura associada seja menor que o limite e 0 caso contrário (1 na condição de solo úmido):
    parametros = dados.umidade_do_solo < limite;

    % Se realiza o cálculo da média de umidade nos últimos 20 dias desconsiderando leituras inválidas:
    media = sum(parametros, 'omitnan') / 2; % Considerando leituras a cada 30 minutos.
    media = 50;
    temperatura = 17;
% Caso a temperatura atual e a umidade ao redor dos últimos 20 dias (19.16 dias para comparação) estejam nas faixas propicias para desenvolvimento dos fungos do gênero Rosellinia spp., logo:
if (temperatura >= 15 && temperatura <= 20) && (media >= 460)
    % Se define a variável cor como vermelho (R=1,G=0,B=0) e a mensagem associada:
    cor = [1 0 0];
    mensagem = "Possível apodrecimento";
% Caso estejam fora das faixas apresentadas, logo:
else
    % Se define a variável cor como verde (R=0,G=1,B=0) e a mensagem associada:
    cor = [0 1 0];
    mensagem = "Condições adequadas";
end

% Abaixo se cria o gráfico utilizado como indicador visual:
figure('Color', 'w'); % Se define uma figura em branco.
hold on; % Mantém a exibição da figura criada.
axis off; % Desativa a exibição dos eixos.
axis equal; % Mantém as mesmas dimensões nos eixos.

% A seguir se desenha o retângulo indicador:
rectangle('Position',[0 0 10 5],'FaceColor',cor,'EdgeColor','black');

% Por fim, se adiciona o texto centralizado com a mensagem
texto = sprintf('%s', mensagem);
text(5, 2.5, texto, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'middle', 'FontSize', 12, 'FontWeight', 'bold', 'Color', 'k');

hold off; % E se desfixa a figura final exibida.
