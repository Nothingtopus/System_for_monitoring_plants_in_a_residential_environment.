% Configurações do canal:
channelID = ---; % Número de identificação do canal.
readAPIKey = '---'; % Chave de leitura no canal, para aquisição de dados.

% Obtém os instantes de início e término do dia, para avaliar a luminosidade diária:
inicio_do_dia = datetime('today');
final_do_dia   = datetime('tomorrow');

% Realiza a obtenção das leituras diárias do sensor de luminosidade e as salva no vetor "dados":
dados = thingSpeakRead(channelID, 'Fields', 4, 'DateRange', [inicio_do_dia final_do_dia], 'ReadKey', readAPIKey, 'OutputFormat', 'table');

% Caso não hajam dados suficientes:
if isempty(dados)
    % Se define a variável cor como branco (R=1,G=1,B=1) e a mensagem associada:
    cor = [1 1 1];
    mensagem = "Sem leituras por hoje";
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
else
    % Estabelece a leitura de 1.65 V ==> 2048 como a associada a exposição solar direta naquele instante:
    limite = 2048;

    % Gera um vetor de "parametros" binários que correspondem a 1 caso a leitura associada seja maior que o limite e 0 caso contrário:
    parametros = dados.luminosidade > limite;
    
    % Testes de funcionamento do widget:
    % Sol pleno:
    %parametros = [1;1;1;1;1;1;1;1;1;1;1;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0];
    % Sol parcial:
    %parametros = [1;1;1;1;1;1;1;1;1;1;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0];
    % Sombra total:
    %parametros = [1;1;1;1;1;1;1;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0];
    
    
    % Se realiza o cálculo da média das horas de exposição solar direta desconsiderando leituras inválidas:
    media = sum(parametros, 'omitnan') / 2; % Considerando leituras a cada 30 minutos.
    
    % Caso a planta tenha recebido 6 horas ou mais de sol direto, logo:
    if media > 6
        % Se define a variável cor como verde (R=0,G=1,B=0) e a mensagem associada:
        cor = [0, 1, 0];
        mensagem = "Sol pleno";
        cor_texto = 'k'; % A cor do texto da mensagem será preto.
    % Caso a planta tenha recebido entre 4 a 6 horas de sol direto, logo:
    elseif media >= 4
        % Se define a variável cor como amarelo (R=1,G=1,B=0) e a mensagem associada:
        cor = [1, 1, 0];
        mensagem = "Sol parcial";
        cor_texto = 'k'; % A cor do texto da mensagem será preto.
    % Caso a planta tenha recebido menos de 4 horas de sol direto, logo:
    else
        % Se define a variável cor como preto (R=0,G=0,B=0) e a mensagem associada:
        cor = [0, 0, 0];
        mensagem = "Sombra total";
        cor_texto = 'w'; % A cor do texto da mensagem será branco.
    end
    % Abaixo se cria o gráfico utilizado como indicador visual:
    figure('Color', 'w'); % Se define uma figura em branco.
    hold on; % Mantém a exibição da figura criada.
    axis off; % Desativa a exibição dos eixos.
    axis equal; % Mantém as mesmas dimensões nos eixos.
    
    % A seguir se desenha o retângulo indicador:
    rectangle('Position',[0 0 10 5],'FaceColor',cor,'EdgeColor','black');
    
    % Por fim, se adiciona o texto centralizado com a mensagem
    texto = sprintf('%.1f Horas\n%s', media, mensagem);
    text(5, 2.5, texto, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'middle', 'FontSize', 12, 'FontWeight', 'bold', 'Color', cor_texto);
    
    hold off; % E se desfixa a figura final exibida.
end
