-- updates_include

DELETE FROM updates_include WHERE path = '$/sql/Bots/updates/world';
INSERT INTO updates_include(path, state) VALUES ('$/sql/Bots/updates/world', 'RELEASED');